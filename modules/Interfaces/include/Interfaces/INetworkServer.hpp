///
/// @file INetworkServer.hpp
/// @brief This file contains the server network interface
/// @namespace srv
///

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "Interfaces/Protocol/Protocol.hpp"
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
            virtual ~INetworkServer() = default;

            // Server lifecycle
            virtual void start() = 0;
            virtual void stop() = 0;

            // Configuration
            virtual void setTickRate(std::uint16_t tickRate) = 0;
            virtual void setServerCapabilities(std::uint32_t caps) = 0;

        private:
    }; // class INetworkServer

} // namespace srv
