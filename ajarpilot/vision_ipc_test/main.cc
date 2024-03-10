#include <thread>
#include <iostream>
#include <chrono>

#include <cereal/visionipc/visionipc_client.h>

int main()
{
    std::cout << "Trying to connect" << std::endl;
    VisionIpcClient client = VisionIpcClient("camerad", VISION_STREAM_DRIVER, false);

    if(!client.connect(false))
    {
        std::cout << "Failed to connect" << std::endl;
        return 1;
    }

    for(int i = 0; i < 10; i++)
    {
        std::cout << "Trying to receive" << std::endl;
        VisionBuf* buf = client.recv();

        if(buf != nullptr)
        {
            std::cout << "Recevied image" << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}