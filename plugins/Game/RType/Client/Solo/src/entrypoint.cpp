#include <memory>

#include "RTypeClientSolo/RTypeClientSolo.hpp"

extern "C"
{
    gme::IGameClient *entryPoint() { return std::make_unique<gme::RTypeClientSolo>().release(); }
}