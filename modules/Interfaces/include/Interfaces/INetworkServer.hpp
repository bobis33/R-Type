///
/// @file INetworkServer.hpp
/// @brief This file contains the server network interface
/// @namespace srv
///

#pragma once

#include "Utils/Interfaces/IPlugin.hpp"

namespace srv
{

    constexpr size_t MAX_CLIENTS = 16;
    constexpr size_t MAX_IP_LENGTH = 8;
    constexpr size_t MAX_LEN_RECV_BUFFER = 1024;

    ///
    /// @class INetworkServer
    /// @brief Interface for the server network
    /// @namespace srv
    ///
    class INetworkServer : public utl::IPlugin
    {
        public:
            virtual void init(uint16_t port, const std::string &address) = 0;
            virtual void start() = 0;
            virtual void stop() = 0;
    }; // class INetworkServer

} // namespace srv
