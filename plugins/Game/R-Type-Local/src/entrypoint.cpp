///
/// @file entrypoint.cpp
/// @brief Entry point for R-Type Local Game plugin
///

#include <memory>
#include "RTypeGameLocal/RTypeGameLocal.hpp"

extern "C"
{
    gme::IGameClient *entryPoint()
    {
        return std::make_unique<gme::RTypeGameLocal>().release();
    }
}

