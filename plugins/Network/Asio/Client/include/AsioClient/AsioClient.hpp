///
/// @file AsioClient.hpp
/// @brief This file contains the client network implementation for Asio
/// @namespace eng
///

#pragma once

#include <string>

#include <asio.hpp>

#include "Interfaces/INetworkClient.hpp"

namespace eng
{

    ///
    /// @class AsioClient
    /// @brief Network implementation with asio for client
    /// @namespace eng
    ///
    class AsioClient final : public INetworkClient
    {
        public:
            AsioClient() = default;
            ~AsioClient() override;

            AsioClient(const AsioClient &) = delete;
            AsioClient(AsioClient &&) = delete;
            AsioClient &operator=(const AsioClient &) = delete;
            AsioClient &operator=(AsioClient &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Network_Asio_Client"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_CLIENT; }

            void connect(const std::string &host, uint16_t port) override;
            void disconnect() override;

        private:
    }; // class AsioClient
} // namespace eng
