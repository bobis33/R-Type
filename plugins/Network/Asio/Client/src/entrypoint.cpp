#include <memory>

#include "AsioClient/AsioClient.hpp"
#include "Utils/Interfaces/IPlugin.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::INetworkClient *entryPoint() { return std::make_unique<eng::AsioClient>().release(); }
}
