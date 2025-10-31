///
/// @file Server.hpp
/// @brief This file contains the Server class declaration
/// @namespace srv
///

#pragma once

#include <memory>

#include "Interfaces/IGameServer.hpp"
#include "Interfaces/INetworkServer.hpp"
#include "Server/ArgsHandler.hpp"
#include "Server/SceneManager.hpp"
#include "Utils/AppConfigServer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/PluginLoader.hpp"

namespace srv
{

    ///
    /// @class Server
    /// @brief Class for the server
    /// @namespace srv
    ///
    class Server
    {

        public:
            explicit Server(const ArgsConfig &config);
            ~Server() = default;

            Server(const Server &) = delete;
            Server &operator=(const Server &) = delete;
            Server(Server &&) = delete;
            Server &operator=(Server &&) = delete;

            void run() const;

        private:
            [[nodiscard]] static utl::srv::AppConfig setupConfig(const ArgsConfig &cfg);

            utl::srv::AppConfig m_config;

            std::unique_ptr<utl::PluginLoader> m_pluginLoader;
            std::unique_ptr<utl::Clock> m_clock;
            std::unique_ptr<SceneManager> m_sceneManager;

            std::shared_ptr<INetworkServer> m_network;
            std::shared_ptr<gme::IGameServer> m_game;
    }; // class Server

} // namespace srv