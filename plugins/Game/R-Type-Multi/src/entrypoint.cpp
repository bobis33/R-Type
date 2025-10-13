///
/// @file entrypoint.cpp
/// @brief Entry point for R-Type Multiplayer Game plugin
///

#include <memory>
#include "RTypeGameMulti/RTypeGameMulti.hpp"

extern "C"
{
    gme::IGameClient *entryPoint()
    {
        return std::make_unique<gme::RTypeGameMulti>().release();
    }
}

