#include <iostream>

#include "NetworkServer/NetworkServer.hpp"

using asio::ip::udp;

srv::NetworkServer::NetworkServer(const unsigned int port, const std::string &address)
    : m_socket(m_ioContext), m_recvBuffer()
{
    const asio::ip::address addr = asio::ip::make_address(address);
    const udp::endpoint ep(addr, static_cast<unsigned short>(port));

    m_socket.open(ep.protocol());
    m_socket.set_option(asio::socket_base::reuse_address(true));
    m_socket.bind(ep);
}

void srv::NetworkServer::start()
{
    m_workGuard.emplace(asio::make_work_guard(m_ioContext));

    startReceive();

    m_ioThread = std::thread(
        [this]
        {
            try
            {
                m_ioContext.run();
            }
            catch (const std::exception &e)
            {
                std::cerr << "[io_context.run] exception: " << e.what() << "\n";
            }
        });
}

void srv::NetworkServer::stop()
{
    if (m_workGuard.has_value())
    {
        asio::post(m_ioContext,
                   [this]
                   {
                       asio::error_code ec;
                       m_socket.close(ec);
                   });

        m_workGuard.reset();
    }

    m_ioContext.stop();

    if (m_ioThread.joinable())
    {
        m_ioThread.join();
    }
}

void srv::NetworkServer::startReceive()
{
    m_socket.async_receive_from(asio::buffer(m_recvBuffer), m_remoteEndpoint,
                                [this](const asio::error_code &ec, const std::size_t n) { handleReceive(ec, n); });
}

void srv::NetworkServer::handleReceive(const asio::error_code &error, const std::size_t bytesTransferred)
{
    if (!error || error == asio::error::message_size)
    {
        std::cout << "UDP from " << m_remoteEndpoint.address().to_string() << ":" << m_remoteEndpoint.port()
                  << " size=" << bytesTransferred << "\n";

        startReceive();
        return;
    }

    if (error == asio::error::operation_aborted)
    {
        return;
    }

    std::cerr << "receive error: " << error.message() << "\n";
    startReceive();
}
