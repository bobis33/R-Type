#pragma once

#include "Interfaces/INetworkServer.hpp"
#include "asio.hpp"
#include <string>
#include <thread>
#include <optional>
#include <iostream>

namespace net {
    class NetworkServer : public srv::INetworkServer {
        public:
            NetworkServer(unsigned int port, const std::string &address);
            ~NetworkServer() override;

            void start() override;
            void stop() override;
        private:

            void startReceive();
            void handleReceive(const asio::error_code& error, std::size_t bytes_transferred);

            asio::io_context _ioContext;
            asio::ip::udp::socket _socket;
            asio::ip::udp::endpoint _remoteEndpoint;
            std::array<char, srv::MAX_LEN_RECV_BUFFER> _recvBuffer;

            std::optional<asio::executor_work_guard<asio::io_context::executor_type>> _workGuard;
            std::thread _ioThread;
    };
}
