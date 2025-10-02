#include <memory>

#include "AsioServer/AsioServer.hpp"

extern "C"
{
    srv::INetworkServer *entryPoint() { return std::make_unique<srv::AsioServer>().release(); }
}