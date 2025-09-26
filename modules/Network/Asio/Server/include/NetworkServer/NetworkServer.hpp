///
/// @file NetworkServer.hpp
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
    /// @class NetworkServer
    /// @brief Network implementation with asio for server
    /// @namespace srv
    ///
    class NetworkServer final : public INetworkServer
    {
        public:
            NetworkServer(unsigned int port, const std::string &address);
            ~NetworkServer() override { stop(); }

            NetworkServer(const NetworkServer &) = delete;
            NetworkServer(NetworkServer &&) = delete;
            NetworkServer &operator=(const NetworkServer &) = delete;
            NetworkServer &operator=(NetworkServer &&) = delete;

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
    }; // class NetworkServer
} // namespace srv
