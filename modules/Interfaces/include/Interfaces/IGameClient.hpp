///
/// @file IGameClient.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include "Engine/Engine.hpp"
#include "Utils/Interfaces/IPlugin.hpp"

namespace gme
{

    ///
    /// @interface IGameClient
    /// @brief Interface for the games
    /// @namespace gme
    ///
    class IGameClient : public utl::IPlugin
    {
        public:
            virtual void init(eng::Engine &engine, float &audioVolume, int skinIndex, bool &showDebug,
                              unsigned int menuSceneId) = 0;
            virtual void update(float deltaTime, unsigned int width, unsigned int height) = 0;
            virtual unsigned int getMainSceneId() const = 0;

        private:
    }; // class IGameClient
} // namespace gme