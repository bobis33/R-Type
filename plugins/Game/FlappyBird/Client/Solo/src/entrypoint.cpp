#include <memory>

#include "FlappyBirdClientSolo/FlappyBirdClientSolo.hpp"

extern "C"
{
    PLUGIN_EXPORT gme::IGameClient *entryPoint() { return std::make_unique<gme::FlappyBirdClientSolo>().release(); }
}