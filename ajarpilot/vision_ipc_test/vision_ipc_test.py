import time

from cereal.visionipc import VisionIpcClient, VisionStreamType

client = VisionIpcClient('camerad', VisionStreamType.VISION_STREAM_DRIVER, False)
client.connect(False)

for i in range(10):
    print("Connecting")
    if(client.connect(False)):
        break

    time.sleep(1)

vis_buf = None

for i in range(10):
    vis_buf = client.recv(False)
    time.sleep(0.1)

if(vis_buf is not None):
    print(vis_buf.width)
    print(vis_buf.height)