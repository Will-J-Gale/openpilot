#!/usr/bin/env python3
import subprocess
import time

import numpy as np
from PIL import Image

import cereal.messaging as messaging
from msgq.visionipc import VisionIpcClient, VisionStreamType
from openpilot.common.realtime import DT_MDL
from openpilot.system.hardware import PC
from openpilot.system.manager.process_config import managed_processes


VISION_STREAMS = {
  "roadCameraState": VisionStreamType.VISION_STREAM_ROAD,
  "driverCameraState": VisionStreamType.VISION_STREAM_DRIVER,
  "wideRoadCameraState": VisionStreamType.VISION_STREAM_WIDE_ROAD,
}

def main():
  time.sleep(2.0)  # Give hardwared time to read the param, or if just started give camerad time to start

  # Check if camerad is already started
  try:
    subprocess.check_call(["pgrep", "camerad"])
    print("Camerad already running")
    return
  except subprocess.CalledProcessError:
    pass

  try:
    # Allow testing on replay on PC
    if not PC:
      managed_processes['camerad'].start()

    sm = messaging.SubMaster(["roadCameraState", "driverCameraState", "wideRoadCameraState"])

    road_camera_client = VisionIpcClient("camerad", VisionStreamType.VISION_STREAM_ROAD, True)
    wide_camera_client = VisionIpcClient("camerad", VisionStreamType.VISION_STREAM_WIDE_ROAD, True)
    driver_camera_client = VisionIpcClient("camerad", VisionStreamType.VISION_STREAM_DRIVER, True)
    vipc_clients = [road_camera_client, wide_camera_client, driver_camera_client]

    # wait 4 sec from camerad startup for focus and exposure
    while sm["wideRoadCameraState"].frameId < int(4. / DT_MDL):
      sm.update()

    for client in vipc_clients:
      client.connect(True)

    # grab images
    while(True):
      capture_start = time.time()

      road_image_buf = road_camera_client.recv()
      wide_image_buf = wide_camera_client.recv()
      driver_image_buf = driver_camera_client.recv()
      dt = time.time() - capture_start
      print(1/dt)

  except KeyboardInterrupt:
    print("Exiting")

  finally:
    managed_processes['camerad'].stop()

if __name__ == "__main__":
  main()
