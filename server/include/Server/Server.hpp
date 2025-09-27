///
/// @file Server.hpp
/// @brief This file contains the Server class declaration
/// @namespace srv
///

#pragma once

#include <memory>

#include "Interfaces/INetworkServer.hpp"
#include "Server/ArgsHandler.hpp"

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
            std::unique_ptr<INetworkServer> m_network;
    }; // class Server

} // namespace srv