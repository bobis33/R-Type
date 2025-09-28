#include <memory>

#include "AsioServer/AsioServer.hpp"

extern "C"
{
    std::unique_ptr<srv::INetworkServer> entryPoint() { return std::make_unique<srv::AsioServer>(); }
}