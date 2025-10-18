#include <memory>

#include "RTypeClientMulti/RTypeClientMulti.hpp"

extern "C"
{
    PLUGIN_EXPORT gme::IGameClient *entryPoint() { return std::make_unique<gme::RTypeClientMulti>().release(); }
}