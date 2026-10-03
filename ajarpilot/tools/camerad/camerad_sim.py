import os
os.environ["PYOPENCL_CTX"] = "0:0"

from argparse import ArgumentParser

import cv2
from openpilot.tools.sim.lib.camerad import Camerad

parser = ArgumentParser()
parser.add_argument("video_path")
parser.add_argument("--loop", action="store_true")

def main(args):
  video = cv2.VideoCapture(args.video_path)
  if(not video.isOpened()):
    raise Exception("Video failed to open")

  camerad = Camerad(False)

  try:
    while(True):
      valid, road_image = video.read()

      if(not valid):
        if(args.loop):
          video.release()
          video = cv2.VideoCapture(args.video_path)
          continue
        else:
          break

      yuv = camerad.rgb_to_yuv(road_image)
      camerad.cam_send_yuv_road(yuv)

  except KeyboardInterrupt:
    pass

  video.release()

if __name__ == "__main__":
  args = parser.parse_args()
  main(args)
