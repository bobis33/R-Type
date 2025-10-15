#include <memory>

#include "RTypeServer/RTypeServer.hpp"

extern "C"
{
    gme::IGameServer *entryPoint() { return std::make_unique<gme::RTypeServer>().release(); }
}