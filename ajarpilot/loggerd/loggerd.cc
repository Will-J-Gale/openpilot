#include <iostream>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <random>
#include <filesystem>

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
#include "emessgee/emessgee.h"

constexpr int MAIN_FPS = 20;
constexpr int SEGMENT_LENGTH = 5; //Seconds
constexpr int FRAMES_PER_SEGMENT = MAIN_FPS * SEGMENT_LENGTH;

const std::string VIDEO_EXT = ".hevc";
const std::filesystem::path DATA_FOLDER = "/data/custom_drives";

ExitHandler do_exit;

std::string create_route_id(int route_count)
{
    std::stringstream route_number;
    route_number << std::setfill('0') << std::setw(8) << std::hex << route_count;

    std::stringstream random_id;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 15);

    for(int i = 0; i < 10; i++)
    {
        random_id << std::hex << dist(mt);
    }

    return route_number.str() + "-" + random_id.str();
}

struct VideoEncoder
{
    std::unique_ptr<VideoWriter> writer;
    bool ready_to_rotate = false;
    size_t frame = -1;
};

//@TODO Put all route file/folder generation in here
// class SegmentHandler
// {

// };

struct LoggerdState
{
    int encoders_ready_to_roate = 0;
    int encoder_count = 0;
    size_t segment = 0;
    std::map<std::string, size_t> encoder_frame;
};

int main(int argc, char** argv)
{
    std::vector<std::string> encoder_names = {
        "roadEncodeData",
        "wideRoadEncodeData"
    };
    std::unique_ptr<Context> ctx(Context::create());

    std::map<std::string, std::unique_ptr<SubSocket>> sockets;
    std::map<std::string, VideoEncoder> video_encoders;

    using EncodeDataGetter = cereal::EncodeData::Reader (cereal::Event::Reader::*)() const;
    std::map<std::string, EncodeDataGetter> get_encode_data_func = {
        {"roadEncodeData", &cereal::Event::Reader::getRoadEncodeData},
        {"wideRoadEncodeData", &cereal::Event::Reader::getWideRoadEncodeData},
    };

    LoggerdState state;

    emessgee::Params params;
    uint32_t route_count = 0;

    if(params.check_key("RouteData"))
    {
        route_count = params.read_int("RouteData");
    }

    params.write_int("RouteData", route_count + 1);

    std::string route_id = create_route_id(route_count);
    std::string segment_path = DATA_FOLDER / (route_id + "-" + std::to_string(state.segment));
    std::filesystem::create_directory(segment_path);

    for(const auto name : encoder_names)
    {
        sockets[name] = std::unique_ptr<SubSocket>(SubSocket::create(ctx.get(), name));
    }
    state.encoder_count = sockets.size();

    while(!do_exit)
    {
        for(const auto&[name, socket] : sockets)
        {
            VideoEncoder& encoder = video_encoders[name];

            Message* msg = nullptr;
            msg = socket->receive(true);

            if(msg != nullptr)
            {
                ++encoder.frame;

                capnp::FlatArrayMessageReader cmsg(kj::ArrayPtr<capnp::word>((capnp::word *)msg->getData(), msg->getSize() / sizeof(capnp::word)));
                auto event = cmsg.getRoot<cereal::Event>();
                auto edata = (event.*(get_encode_data_func[name]))();
                auto idx = edata.getIdx();
                auto flags = idx.getFlags();
                auto header = edata.getHeader();

                if(encoder.frame >= FRAMES_PER_SEGMENT)
                {
                    encoder.writer.reset();
                    encoder.frame = 0;
                    encoder.ready_to_rotate = true;
                    ++state.encoders_ready_to_roate;
                }

                if(!encoder.writer && flags & V4L2_BUF_FLAG_KEYFRAME)
                {
                    encoder.writer = std::make_unique<VideoWriter>(
                        segment_path.c_str(),
                        (name + VIDEO_EXT).c_str(),
                        false,
                        edata.getWidth(),
                        edata.getHeight(),
                        MAIN_FPS,
                        cereal::EncodeIndex::Type::FULL_H_E_V_C
                    );

                    encoder.writer->write((uint8_t *)header.begin(), header.size(), idx.getTimestampEof()/1000, true, false);
                }

                if(encoder.writer)
                {
                    auto data = edata.getData();
                    encoder.writer->write((uint8_t *)data.begin(), data.size(), idx.getTimestampEof()/1000, false, flags & V4L2_BUF_FLAG_KEYFRAME);
                }

                delete msg;
            }
        }

        //Create new segment
        if(state.encoders_ready_to_roate >= sockets.size())
        {
            state.encoders_ready_to_roate = 0;
            state.segment++;
            segment_path = DATA_FOLDER / (route_id + "-" + std::to_string(state.segment));
            std::filesystem::create_directory(segment_path);
        }
    }

    return 0;
}