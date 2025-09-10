///
/// @file Server.hpp
/// @brief This file contains the Server class declaration
/// @namespace rtp
///

#pragma once

#include "R-Type/ArgsHandler.hpp"

namespace rtp
{

    ///
    /// @class Server
    /// @brief Class for the server
    /// @namespace rtp
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

        private:
    }; // class Server

} // namespace rtp