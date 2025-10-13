///
/// @file entrypoint.cpp
/// @brief Entry point for R-Type Game plugin
///

#include <memory>
#include "RTypeGame/RTypeGame.hpp"

extern "C"
{
    gme::IGameClient *entryPoint()
    {
        return std::make_unique<gme::RTypeGame>().release();
    }
}

