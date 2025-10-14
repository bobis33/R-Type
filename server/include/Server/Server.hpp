///
/// @file Server.hpp
/// @brief This file contains the Server class declaration
/// @namespace srv
///

#pragma once

#include <memory>

#include "Interfaces/INetworkServer.hpp"
#include "Server/ArgsHandler.hpp"
#include "Utils/PluginLoader.hpp"

namespace srv
{

    struct AppConfig
    {
        std::string host;
        uint16_t port;
    };

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
            AppConfig setupConfig(const ArgsConfig &cfg) const;

            AppConfig m_config;

            std::unique_ptr<utl::PluginLoader> m_pluginLoader;
            std::shared_ptr<INetworkServer> m_network;
    }; // class Server

} // namespace srv