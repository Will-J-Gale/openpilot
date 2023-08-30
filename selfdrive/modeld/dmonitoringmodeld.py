#!/usr/bin/env python3
import os
import gc
import math
import time
import ctypes
import numpy as np
from pathlib import Path
from typing import Tuple, Dict
from cereal import messaging
from cereal.messaging import PubMaster, SubMaster
from cereal.visionipc import VisionIpcClient, VisionStreamType, VisionBuf
from openpilot.system.hardware import TICI
from openpilot.system.swaglog import cloudlog
from openpilot.common.params import Params
from openpilot.selfdrive.modeld.models.commonmodel_pyx import Runtime

USE_SNPE_MODEL = TICI or int(os.getenv('USE_SNPE_MODEL', '0'))
if USE_SNPE_MODEL:
  os.environ['ADSP_LIBRARY_PATH'] = "/data/pythonpath/third_party/snpe/dsp/"
  from selfdrive.modeld.runners.snpemodel_pyx import SNPEModel as ModelRunner
else:
  from selfdrive.modeld.runners.onnxmodel_pyx import ONNXModel as ModelRunner

CALIB_LEN = 3
REG_SCALE = 0.25
MODEL_WIDTH = 1440
MODEL_HEIGHT = 960
OUTPUT_SIZE = 84
SEND_RAW_PRED = os.getenv('SEND_RAW_PRED')
MODEL_PATH = str(Path(__file__).parent / 'models' / ('dmonitoring_model_q.dlc' if USE_SNPE_MODEL else 'dmonitoring_model.onnx'))

class DriverStateResult(ctypes.Structure):
  _fields_ = [
    ("face_orientation", ctypes.c_float*3),
    ("face_orientation_std", ctypes.c_float*3),
    ("face_position", ctypes.c_float*2),
    ("face_position_std", ctypes.c_float*2),
    ("face_prob", ctypes.c_float),
    ("left_eye_prob", ctypes.c_float),
    ("right_eye_prob", ctypes.c_float),
    ("left_blink_prob", ctypes.c_float),
    ("right_blink_prob", ctypes.c_float),
    ("sunglasses_prob", ctypes.c_float),
    ("occluded_prob", ctypes.c_float),
    ("ready_prob", ctypes.c_float*4),
    ("not_ready_prob", ctypes.c_float*2)]

class DMonitoringModelResult(ctypes.Structure):
  _fields_ = [
    ("driver_state_lhd", DriverStateResult),
    ("driver_state_rhd", DriverStateResult),
    ("poor_vision_prob", ctypes.c_float),
    ("wheel_on_right_prob", ctypes.c_float)]

class ModelState:
  inputs: Dict[str, np.ndarray]
  output: np.ndarray
  model: ModelRunner

  def __init__(self):
    assert ctypes.sizeof(DMonitoringModelResult) == OUTPUT_SIZE * ctypes.sizeof(ctypes.c_float)
    self.output = np.zeros(OUTPUT_SIZE, dtype=np.float32)
    self.inputs = {
      'input_imgs': np.zeros(MODEL_HEIGHT * MODEL_WIDTH, dtype=np.uint8),
      'calib': np.zeros(CALIB_LEN, dtype=np.float32)}
    self.model = ModelRunner(MODEL_PATH, self.output, Runtime.DSP, True, None)
    self.model.addInput("input_imgs", None)
    self.model.addInput("calib", self.inputs['calib'])

  def run(self, buf:VisionBuf) -> Tuple[np.ndarray, float]:
    v_offset = buf.height - MODEL_HEIGHT
    h_offset = (buf.width - MODEL_WIDTH) / 2
    buf_data = buf.data

    # make a uint8 copy
    for row in range(MODEL_HEIGHT):
      dst_offset = row * MODEL_WIDTH
      src_offset = (v_offset + row) * buf.stride + h_offset
      self.inputs['input_img'][dst_offset:dst_offset+MODEL_WIDTH] = buf_data[src_offset:src_offset+MODEL_WIDTH]

    t1 = time.perf_counter()
    self.model.setInputBuffer("input_imgs", self.inputs['input_imgs'].view(np.float32))
    self.model.execute()
    t2 = time.perf_counter()
    return self.output, t2 - t1

def sigmoid(x):
  return 1. / (1. + math.exp(-x))

def get_driver_state(ds_result: DriverStateResult):
  return {
    "faceOrientation": [x * REG_SCALE for x in ds_result.face_orientation],
    "faceOrientationStd": [math.exp(x) for x in ds_result.face_orientation_std],
    "facePosition": [x * REG_SCALE for x in ds_result.face_position],
    "facePositionStd": [math.exp(x) for x in ds_result.face_position_std],
    "faceProb": sigmoid(ds_result.face_prob),
    "leftEyeProb": sigmoid(ds_result.left_eye_prob),
    "rightEyeProb": sigmoid(ds_result.right_eye_prob),
    "leftBlinkProb": sigmoid(ds_result.left_blink_prob),
    "rightBlinkProb": sigmoid(ds_result.right_blink_prob),
    "sunglassesProb": sigmoid(ds_result.sunglasses_prob),
    "occludedProb": sigmoid(ds_result.occluded_prob),
    "readyProb": [sigmoid(x) for x in ds_result.ready_prob],
    "notReadyProb": [sigmoid(x) for x in ds_result.not_ready_prob]}

def get_driverstate_packet(model_output: np.ndarray, frame_id: int, location_ts: int, execution_time: float, dsp_execution_time: float):
  model_result = model_output.ctypes.data_as(ctypes.POINTER(DMonitoringModelResult)).contents
  msg = messaging.new_message('driverStateV2')
  msg.driverStateV2 = {
    'frameId': frame_id,
    'modelExecutionTime': execution_time,
    'dspExecutionTime': dsp_execution_time,
    'poorVisionProb': sigmoid(model_result.poor_vision_prob),
    'wheelOnRightProb': sigmoid(model_result.wheel_on_right_prob),
    'driver_state_lhd': get_driver_state(model_result.driver_state_lhd),
    'driver_state_rhd': get_driver_state(model_result.driver_state_rhd),
    'rawPredictions': model_output.tobytes() if SEND_RAW_PRED else None}

  return msg


def main():
  gc.disable()
  os.setpriority(os.PRIO_PROCESS, 0, -15)

  model = ModelState()
  cloudlog.warning("models loaded, dmonitoringmodeld starting")

  Params().put_bool("DmModelInitialized", True)

  cloudlog.warning("connecting to driver stream")
  vipc_client = VisionIpcClient("camerad", VisionStreamType.VISION_STREAM_DRIVER, True)
  while not vipc_client.connect(False):
    time.sleep(0.1)
  assert vipc_client.is_connected()
  cloudlog.warning(f"connected with buffer size: {vipc_client.buffer_len}")

  sm = SubMaster(["liveCalibration"])
  pm = PubMaster(["driverStateV2"])

  calib = np.zeros(CALIB_LEN, dtype=np.float32)
  # last = 0

  while True:
    buf = vipc_client.recv()
    if buf is None:
      continue

    sm.update(0)
    if sm.updated["liveCalibration"]:
      calib[:] = np.array(sm["liveCalibration"].rpyCalib)

    t1 = time.perf_counter()
    model_output, dsp_execution_time = model.run(buf)
    t2 = time.perf_counter()

    pm.send("driverStateV2", get_driverstate_packet(model_output, vipc_client.frame_id, vipc_client.timestamp_sof, t2 - t1, dsp_execution_time))
    # print("dmonitoring process: %.2fms, from last %.2fms\n" % (t2 - t1, t1 - last))
    # last = t1


if __name__ == "__main__":
  main()