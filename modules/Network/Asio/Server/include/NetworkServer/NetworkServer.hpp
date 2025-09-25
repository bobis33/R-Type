#pragma once

#include "Interfaces/INetworkServer.hpp"
#include "asio.hpp"
#include <string>
#include <thread>
#include <optional>

namespace srv {

    class NetworkServer final : public INetworkServer {
        public:
            NetworkServer(unsigned int port, const std::string &address);
            ~NetworkServer() override;

            void start() override;
            void stop() override;
        private:

            void startReceive();
            void handleReceive(const asio::error_code& error, std::size_t bytes_transferred);

            asio::io_context m_ioContext;
            asio::ip::udp::socket m_socket;
            asio::ip::udp::endpoint m_remoteEndpoint;
            std::array<char, MAX_LEN_RECV_BUFFER> m_recvBuffer;

            std::optional<asio::executor_work_guard<asio::io_context::executor_type>> m_workGuard;
            std::thread m_ioThread;
    };
}
