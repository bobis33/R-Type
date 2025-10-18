#include <memory>

#include "AsioServer/AsioServer.hpp"

extern "C"
{
    PLUGIN_EXPORT srv::INetworkServer *entryPoint() { return std::make_unique<srv::AsioServer>().release(); }
}