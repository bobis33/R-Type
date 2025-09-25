#include "NetworkServer/NetworkServer.hpp"

using asio::ip::udp;

namespace net {

NetworkServer::NetworkServer(unsigned int port, const std::string &address)
: _ioContext()
, _socket(_ioContext)
{
    asio::ip::address addr = asio::ip::make_address(address);
    udp::endpoint ep(addr, static_cast<unsigned short>(port));

    _socket.open(ep.protocol());
    _socket.set_option(asio::socket_base::reuse_address(true));
    _socket.bind(ep);
}

NetworkServer::~NetworkServer() {
    stop();
}

void NetworkServer::start() {
    _workGuard.emplace(asio::make_work_guard(_ioContext));

    startReceive();

    _ioThread = std::thread([this]{
        try {
            _ioContext.run();
        } catch (const std::exception& e) {
            std::cerr << "[io_context.run] exception: " << e.what() << "\n";
        }
    });
}

void NetworkServer::stop() {
    if (_workGuard.has_value()) {
        asio::post(_ioContext, [this]{
            asio::error_code ec;
            _socket.cancel(ec);
            _socket.close(ec);
        });

        _workGuard.reset();
    }

    _ioContext.stop();

    if (_ioThread.joinable()) {
        _ioThread.join();
    }

    _ioContext.restart();
}

void NetworkServer::startReceive() {
    _socket.async_receive_from(
        asio::buffer(_recvBuffer),
        _remoteEndpoint,
        [this](const asio::error_code& ec, std::size_t n) {
            handleReceive(ec, n);
        }
    );
}

void NetworkServer::handleReceive(const asio::error_code& error, std::size_t bytes_transferred) {
    if (!error || error == asio::error::message_size) {
        std::cout << "UDP from " << _remoteEndpoint.address().to_string()
                  << ":" << _remoteEndpoint.port()
                  << " size=" << bytes_transferred << "\n";

        startReceive();
        return;
    }

    if (error == asio::error::operation_aborted) {
        return;
    }

    std::cerr << "receive error: " << error.message() << "\n";
    startReceive();
}

} // namespace net
