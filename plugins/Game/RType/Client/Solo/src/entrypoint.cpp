#include <memory>

#include "RTypeClientSolo/RTypeClientSolo.hpp"

extern "C"
{
    PLUGIN_EXPORT gme::IGameClient *entryPoint() { return std::make_unique<gme::RTypeClientSolo>().release(); }
}