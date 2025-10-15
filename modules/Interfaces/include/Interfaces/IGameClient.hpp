///
/// @file IGameClient.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include "Utils/Interfaces/IPlugin.hpp"

namespace gme
{

    ///
    /// @class IGameClient
    /// @brief Interface for the games
    /// @namespace gme
    ///
    class IGameClient : public utl::IPlugin
    {
        public:

            virtual void update(float deltaTime, unsigned int width, unsigned int height) = 0;

        private:
    }; // class IGameClient
} // namespace gme