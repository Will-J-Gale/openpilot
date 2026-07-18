#include <iostream>
#include <memory>
#include <map>
#include <string>

// has to be in this order
#ifdef __linux__
#include "third_party/linux/include/v4l2-controls.h"
#include <linux/videodev2.h>
#else
#define V4L2_BUF_FLAG_KEYFRAME 8
#endif

#include "msgq/ipc.h"
#include "common/util.h"
#include "system/loggerd/video_writer.h"
#include "cereal/messaging/messaging.h"

constexpr int MAIN_FPS = 20;
const std::string VIDEO_EXT = ".hevc";

ExitHandler do_exit;

int main(int argc, char** argv)
{
    std::vector<std::string> encoder_names = {
        "roadEncodeData",
        "wideRoadEncodeData"
    };
    std::unique_ptr<Context> ctx(Context::create());

    std::map<std::string, std::unique_ptr<SubSocket>> sockets;
    std::map<std::string, std::unique_ptr<VideoWriter>> video_writers;

    using EncodeDataGetter = cereal::EncodeData::Reader (cereal::Event::Reader::*)() const;
    std::map<std::string, EncodeDataGetter> get_encode_data_func = {
        {"roadEncodeData", &cereal::Event::Reader::getRoadEncodeData},
        {"wideRoadEncodeData", &cereal::Event::Reader::getWideRoadEncodeData},
    };

    for(const auto name : encoder_names)
    {
        sockets[name] = std::unique_ptr<SubSocket>(SubSocket::create(ctx.get(), name));
    }

    while(!do_exit)
    {
        for(const auto&[name, socket] : sockets)
        {
            Message* msg = nullptr;
            msg = socket->receive(true);

            if(msg != nullptr)
            {
                capnp::FlatArrayMessageReader cmsg(kj::ArrayPtr<capnp::word>((capnp::word *)msg->getData(), msg->getSize() / sizeof(capnp::word)));
                auto event = cmsg.getRoot<cereal::Event>();
                auto edata = (event.*(get_encode_data_func[name]))();
                auto idx = edata.getIdx();
                auto flags = idx.getFlags();
                auto header = edata.getHeader();

                if(video_writers.count(name) == 0 && flags & V4L2_BUF_FLAG_KEYFRAME)
                {
                    video_writers[name] = std::make_unique<VideoWriter>(
                        "/data/custom_drives",
                        (name + VIDEO_EXT).c_str(),
                        false,
                        edata.getWidth(),
                        edata.getHeight(),
                        MAIN_FPS,
                        cereal::EncodeIndex::Type::FULL_H_E_V_C
                    );

                    video_writers[name]->write((uint8_t *)header.begin(), header.size(), idx.getTimestampEof()/1000, true, false);
                }

                auto data = edata.getData();
                video_writers[name]->write((uint8_t *)data.begin(), data.size(), idx.getTimestampEof()/1000, false, flags & V4L2_BUF_FLAG_KEYFRAME);
            }
        }
    }

    return 0;
}