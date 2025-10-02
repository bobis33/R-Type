///
/// @file AsioServer.hpp
/// @brief This file contains the server network implementation for Asio
/// @namespace srv
///

#pragma once

#include <optional>
#include <string>
#include <thread>

#include <asio.hpp>

#include "Interfaces/INetworkServer.hpp"

namespace srv
{

    ///
    /// @class AsioServer
    /// @brief Network implementation with asio for server
    /// @namespace srv
    ///
    class AsioServer final : public INetworkServer
    {
        public:
            AsioServer() : m_socket(m_ioContext), m_recvBuffer() {}
            ~AsioServer() override { stop(); }

            AsioServer(const AsioServer &) = delete;
            AsioServer(AsioServer &&) = delete;
            AsioServer &operator=(const AsioServer &) = delete;
            AsioServer &operator=(AsioServer &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Network_Asio_Server"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::NETWORK_SERVER; }

            void init(uint16_t port, const std::string &address) override;
            void start() override;
            void stop() override;

        private:
            void startReceive();
            void handleReceive(const asio::error_code &error, std::size_t bytesTransferred);

            asio::io_context m_ioContext;
            asio::ip::udp::socket m_socket;
            asio::ip::udp::endpoint m_remoteEndpoint;
            std::array<char, MAX_LEN_RECV_BUFFER> m_recvBuffer;

            std::optional<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;
            std::thread m_ioThread;
    }; // class AsioServer
} // namespace srv
