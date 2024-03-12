#include <thread>
#include <iostream>
#include <chrono>
#include <memory>

#include <cereal/visionipc/visionipc_client.h>
#include <system/loggerd/logger.h>

#ifdef QCOM2
#include "system/loggerd/encoder/v4l_encoder.h"
#define Encoder V4LEncoder
#else
#include "system/loggerd/encoder/ffmpeg_encoder.h"
#define Encoder FfmpegEncoder
#endif

int main()
{
    std::cout << "Trying to connect" << std::endl;
    VisionIpcClient client = VisionIpcClient("camerad", VISION_STREAM_DRIVER, false);
    std::unique_ptr<Encoder> encoder;
    const EncoderInfo encoder_info = {
        .publish_name = "wideRoadEncodeData",
        .filename = "test.hevc",
        INIT_ENCODE_FUNCTIONS(WideRoadEncode),
    }; 

    if(!client.connect(false))
    {
        std::cout << "Failed to connect" << std::endl;
        return 1;
    }

    for(int i = 0; i < 100; i++)
    {
        std::cout << "Trying to receive" << std::endl;
        VisionIpcBufExtra extra;
        VisionBuf* buf = client.recv(&extra);

        if(buf != nullptr)
        {
            encoder.reset(new Encoder(encoder_info, buf->width, buf->height));
            encoder->encode_frame(buf, &extra);
            std::cout << "Recevied image" << std::endl;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    return 0;
}