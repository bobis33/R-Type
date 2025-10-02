#include <memory>

#include "AsioClient/AsioClient.hpp"

extern "C"
{
    eng::INetworkClient *entryPoint() { return std::make_unique<eng::AsioClient>().release(); }
}