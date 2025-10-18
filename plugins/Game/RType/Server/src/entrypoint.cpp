#include <memory>

#include "RTypeServer/RTypeServer.hpp"

extern "C"
{
    PLUGIN_EXPORT gme::IGameServer *entryPoint() { return std::make_unique<gme::RTypeServer>().release(); }
}