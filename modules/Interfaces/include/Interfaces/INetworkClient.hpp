///
/// @file INetworkClient.hpp
/// @brief This file contains the client network interface
/// @namespace eng
///

#pragma once

#include <cstdint>
#include <string>

#include "Utils/Interfaces/IPlugin.hpp"

namespace eng
{

    ///
    /// @brief Connection state enumeration
    ///
    enum class ConnectionState
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING
    };

    ///
    /// @class INetworkClient
    /// @brief Interface for the client network
    /// @namespace eng
    ///
    class INetworkClient : public utl::IPlugin
    {
        public:
            virtual ~INetworkClient() = default;

            // Connection management
            virtual void connect(const std::string &host, uint16_t port) = 0;
            virtual void disconnect() = 0;

            // Packet handling
            virtual void update() = 0;

            // Connection status
            [[nodiscard]] virtual bool isConnected() const = 0;
            [[nodiscard]] virtual ConnectionState getConnectionState() const = 0;
            [[nodiscard]] virtual std::uint32_t getSessionId() const = 0;
            [[nodiscard]] virtual std::uint16_t getServerTickRate() const = 0;
            [[nodiscard]] virtual std::uint32_t getLatency() const = 0;

            // Client configuration
            virtual void setPlayerName(const std::string &playerName) = 0;
            virtual void setClientCapabilities(std::uint32_t caps) = 0;

        private:
    }; // class INetworkClient

} // namespace eng
