#include <memory>

#include "AsioClient/AsioClient.hpp"

extern "C"
{
    std::unique_ptr<eng::INetworkClient> entryPoint() { return std::make_unique<eng::AsioClient>(); }
}