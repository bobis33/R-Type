///
/// @file IGameServer.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include "ECS/Entity.hpp"
#include "ECS/Registry.hpp"
#include "Utils/Interfaces/IPlugin.hpp"

namespace gme
{


    enum class State : uint8_t
    {
        PLAYING = 0,
        WIN = 1,
        LOSE = 2,
    };

    ///
    /// @class IGameServer
    /// @brief Interface for the games
    /// @namespace gme
    ///
    class IGameServer : public utl::IPlugin
    {
        public:
            [[nodiscard]] virtual State getState() const = 0;

            virtual void start() = 0;
            virtual void stop() = 0;
            virtual void update(float deltaTime) = 0;

        private:
    }; // class IGameServer

} // namespace gme