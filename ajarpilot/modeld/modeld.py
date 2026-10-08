#!/usr/bin/env python3

import time
from pathlib import Path

import numpy as np

from msgq.visionipc import VisionIpcClient, VisionStreamType, VisionBuf
from openpilot.selfdrive.modeld.models.commonmodel_pyx import DrivingModelFrame, CLContext
from openpilot.selfdrive.modeld.runners.ort_helpers import make_onnx_cpu_runner
from openpilot.system.hardware import TICI
from ajarpilot.messaging import log
from ajarpilot.emessgee.emessgee import Publisher

MODEL_PATH = Path(__file__).parent / "models/entnet.onnx"
MODEL_PKL_PATH = Path(__file__).parent / "models/entnet_tinygrad.pkl"

#PUT IN METADATA DICT?
INPUT_SHAPE = (1, 12, 128, 256)
PATH_MEANSTD = np.array([-0.2636184, -2.49679575, 73.66905499], dtype=np.float32), np.array([5.97214545, 1.64058577, 43.01043996], dtype=np.float32)
PATH_MODES = 3
PATH_POINTS = 34
PATH_SLICE = slice(0, 615)
LEAD_CAR_MEANSTD = np.array([-0.016811527037770713, -1.2692863468207345, 37.99210456906899]), np.array([1.4749043363077345, 0.7072242920228887, 19.954947472647078])
LEAD_CAR_SLICE = slice(615, 619)

def sigmoid(value):
    return 1 / (1 + np.exp(-value))

def safe_exp(x, max_value=88): #exp(88) approx max float32 value before overflow
    return np.exp(np.clip(x, -np.inf, max_value))

#From openpilot
def softmax(x, axis=-1):
  x -= np.max(x, axis=axis, keepdims=True)
  if x.dtype == np.float32 or x.dtype == np.float64:
    x = safe_exp(x)
  else:
    x = safe_exp(x)
  x /= np.sum(x, axis=axis, keepdims=True)
  return x

def parse_model_output(prediction:np.ndarray, path_meanstd:np.ndarray, lead_car_meanstd:np.ndarray) -> dict:
    #Path
    path_pred = prediction[PATH_SLICE].reshape(3, PATH_POINTS*6+1).copy()
    path_class_pred = path_pred[:, 0]
    index = np.argmax(softmax(path_class_pred, axis=0))
    path = path_pred[index, 1:].reshape(PATH_POINTS, 6)
    mean, std = path_meanstd
    path[..., :3]  = (path[..., :3] * std) + mean
    path[..., 3:] = safe_exp(path[..., 3:])

    #Lead car
    mean, std = lead_car_meanstd
    lead_car = prediction[LEAD_CAR_SLICE].copy()
    lead_car[0] = sigmoid(lead_car[0])
    lead_car[1:] = (lead_car[1:] * std) + mean

    model_data = log.ModelData.new_message()

    model_data.init("path", 34)
    for i, pos in enumerate(path):
        x,y,z,x_std,y_std,z_std = pos.tolist()
        model_data.path[i].x = x
        model_data.path[i].y = y
        model_data.path[i].z = z
        model_data.path[i].xStd = x_std
        model_data.path[i].yStd = y_std
        model_data.path[i].zStd = z_std

    model_data.init("leadCar", 4)
    model_data.leadCar = lead_car.tolist()

    return model_data

class OnnxModel:
    def __init__(self, model_path:str, cl_context:CLContext):
        self.model = make_onnx_cpu_runner(model_path)
        self.frames = DrivingModelFrame(cl_context)

    def run(self, buf:VisionBuf, transform: np.ndarray):
        images_cl = self.frames.prepare(buf, transform.flatten())
        model_inputs = self.frames.buffer_from_cl(images_cl).reshape(INPUT_SHAPE).astype(dtype=np.uint8)
        return self.model.run(None, {"input_img": model_inputs})

#@TODO Figure this out on comma
class TinygradModel:
    def __init__(self, model_path):
        pass

    def run(self, buf:VisionBuf, transform: np.ndarray):
        pass

def main():
    cl_context = CLContext()
    vipc_client = VisionIpcClient("camerad", VisionStreamType.VISION_STREAM_ROAD, True, cl_context)
    transform = np.array([
        [2.0,   0.0, 452.0],
        [0.0,   2.0, 348.0],
        [0.0,   0.0,   1.0]
    ], dtype=np.float32)

    pub = Publisher(["model_data"], 1e4, 10)

    if(TICI):
        model = TinygradModel(MODEL_PKL_PATH)
    else:
        model = OnnxModel(MODEL_PATH, cl_context)

    while not vipc_client.connect(False):
        time.sleep(0.1)

    while(True):
        try:
          buf = vipc_client.recv()

          if(buf is None):
              continue

          prediction = model.run(buf, transform)[0].flatten()
          model_data = parse_model_output(prediction, PATH_MEANSTD, LEAD_CAR_MEANSTD)
          pub.send("model_data", model_data.to_bytes_packed())
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()