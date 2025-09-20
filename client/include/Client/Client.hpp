///
/// @file Client.hpp
/// @brief This file contains the Client class declaration
/// @namespace cli
///

#pragma once

#include <unordered_map>

#include "Client/ArgsHandler.hpp"
#include "Engine/Engine.hpp"
#include "Interfaces/IGameClient.hpp"
#include "Utils/Clock.hpp"

namespace cli
{

    ///
    /// @class Client
    /// @brief Class for the client
    /// @namespace cli
    ///
    class Client
    {

        public:
            explicit Client(const ArgsConfig &cfg);
            ~Client() = default;

            Client(const Client &) = delete;
            Client &operator=(const Client &) = delete;
            Client(Client &&) = delete;
            Client &operator=(Client &&) = delete;

            void run();

        private:
            void syncEntitiesToECS();

            ecs::Entity m_fpsEntity = 0;
            std::unique_ptr<gme::IGameClient> m_game;
            std::unique_ptr<eng::Engine> m_engine;
            std::unordered_map<std::string, ecs::Entity> m_entityMap;
    }; // class Client

} // namespace cli