#include <cstring>
#include <iostream>

#include "AsioServer/AsioServer.hpp"

using asio::ip::udp;

srv::AsioServer::AsioServer(const uint16_t port, const std::string &address) : m_socket(m_ioContext), m_recvBuffer()
{
    const asio::ip::address addr = asio::ip::make_address(address);
    const udp::endpoint ep(addr, port);

    m_socket.open(ep.protocol());
    m_socket.set_option(asio::socket_base::reuse_address(true));
    m_socket.bind(ep);
}

void srv::AsioServer::start()
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
                std::cerr << "[AsioServer] IO context exception: " << e.what() << "\n";
            }
        });
}

void srv::AsioServer::stop()
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

void srv::AsioServer::startReceive()
{
    m_socket.async_receive_from(asio::buffer(m_recvBuffer), m_remoteEndpoint,
                                [this](const asio::error_code &ec, const std::size_t n) { handleReceive(ec, n); });
}

void srv::AsioServer::handleReceive(const asio::error_code &error, const std::size_t bytesTransferred)
{
    if (!error)
    {
        std::vector<uint8_t> data(m_recvBuffer.begin(), m_recvBuffer.begin() + bytesTransferred);
        processPacket(m_remoteEndpoint, data);
        startReceive();
    }
    else if (error == asio::error::operation_aborted)
    {
        return;
    }
    else
    {
        std::cerr << "[AsioServer] Erreur de réception: " << error.message() << "\n";
        startReceive();
    }
}

void srv::AsioServer::handleSend(const asio::error_code &error, std::size_t bytesTransferred)
{
    if (error)
    {
        std::cerr << "[AsioServer] Erreur d'envoi: " << error.message() << "\n";
    }
}

void srv::AsioServer::processPacket(const asio::ip::udp::endpoint &sender, const std::vector<uint8_t> &data)
{
    try
    {
        rnp::PacketHeader header = rnp::deserializeHeader(data.data(), data.size());

        std::vector<uint8_t> payload;
        if (header.length > 0 && data.size() >= sizeof(rnp::PacketHeader) + header.length)
        {
            payload.assign(data.begin() + sizeof(rnp::PacketHeader),
                           data.begin() + sizeof(rnp::PacketHeader) + header.length);
        }

        // Vérifier la version du protocole
        if (header.version != rnp::PROTOCOL_VERSION)
        {
            sendError(sender, "Version de protocole non supportée");
            return;
        }

        switch (header.type)
        {
            case rnp::PacketType::CONNECT:
            {
                if (payload.size() >= 1)
                {
                    std::string playerName(payload.begin(), payload.end());
                    playerName.erase(std::find(playerName.begin(), playerName.end(), '\0'), playerName.end());
                    addClient(sender, playerName);
                    std::cout << "[AsioServer] Client connecté: " << playerName << " (" << sender.address().to_string()
                              << ":" << sender.port() << ")\n";
                }
                break;
            }
            case rnp::PacketType::DISCONNECT:
            {
                removeClient(sender);
                std::cout << "[AsioServer] Client déconnecté: " << sender.address().to_string() << ":" << sender.port()
                          << "\n";
                break;
            }
            case rnp::PacketType::PLAYER_INPUT:
            {
                if (payload.size() >= 2)
                {
                    const std::uint8_t direction = payload[0];
                    const std::uint8_t shooting = payload[1];

                    rnp::EventRecord ev;
                    ev.type = rnp::EventType::INPUT;
                    ev.data.reserve(4);

                    const std::uint16_t playerId = getPlayerId(sender);
                    ev.data.push_back(static_cast<std::uint8_t>(playerId & 0xFF));
                    ev.data.push_back(static_cast<std::uint8_t>((playerId >> 8) & 0xFF));
                    ev.data.push_back(direction);
                    ev.data.push_back(shooting);

                    std::vector<rnp::EventRecord> batch;
                    batch.emplace_back(std::move(ev));
                    broadcastEvents(batch);
                }
                break;
            }
            case rnp::PacketType::PING:
            {
                sendPong(sender);
                break;
            }
            default:
                break;
        }

        auto it = m_packetHandlers.find(header.type);
        if (it != m_packetHandlers.end())
        {
            it->second(sender, header, payload);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AsioServer] Erreur de traitement du paquet: " << e.what() << "\n";
        sendError(sender, "Erreur de traitement du paquet");
    }
}

void srv::AsioServer::addClient(const asio::ip::udp::endpoint &endpoint, const std::string &playerName)
{
    ClientInfo info;
    info.endpoint = endpoint;
    info.playerName = playerName;
    info.lastSequence = 0;
    info.connected = true;
    info.playerId = m_nextPlayerId++;
    m_clients[endpoint] = info;
}

void srv::AsioServer::removeClient(const asio::ip::udp::endpoint &endpoint) { m_clients.erase(endpoint); }

std::uint16_t srv::AsioServer::getPlayerId(const asio::ip::udp::endpoint &endpoint) const
{
    auto it = m_clients.find(endpoint);
    if (it != m_clients.end())
    {
        return it->second.playerId;
    }
    return 0;
}

void srv::AsioServer::sendWorldState(const asio::ip::udp::endpoint &client, const std::vector<uint8_t> &worldData)
{
    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::WORLD_STATE;
    header.length = worldData.size();
    header.sequence = ++m_sequenceNumber;

    std::vector<uint8_t> buffer = rnp::serialize(header, worldData.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendEvents(const asio::ip::udp::endpoint &client, const std::vector<rnp::EventRecord> &events)
{
    const std::vector<uint8_t> eventsPayload = rnp::serializeEvents(events);

    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::EVENTS;
    header.length = static_cast<std::uint16_t>(eventsPayload.size());
    header.sequence = ++m_sequenceNumber;

    std::vector<uint8_t> buffer = rnp::serialize(header, eventsPayload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendPong(const asio::ip::udp::endpoint &client)
{
    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::PONG;
    header.length = 0;
    header.sequence = ++m_sequenceNumber;

    std::vector<uint8_t> buffer = rnp::serialize(header);

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendError(const asio::ip::udp::endpoint &client, const std::string &errorMessage)
{
    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::ERROR;
    header.length = errorMessage.size();
    header.sequence = ++m_sequenceNumber;

    std::vector<uint8_t> buffer = rnp::serialize(header, reinterpret_cast<const uint8_t *>(errorMessage.c_str()));

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::broadcastToAll(const std::vector<uint8_t> &data)
{
    for (const auto &[endpoint, clientInfo] : m_clients)
    {
        if (clientInfo.connected)
        {
            m_socket.async_send_to(asio::buffer(data), endpoint,
                                   [this](const asio::error_code &error, std::size_t bytesTransferred)
                                   { handleSend(error, bytesTransferred); });
        }
    }
}

void srv::AsioServer::broadcastEvents(const std::vector<rnp::EventRecord> &events)
{
    const std::vector<uint8_t> payload = rnp::serializeEvents(events);

    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::EVENTS;
    header.length = static_cast<std::uint16_t>(payload.size());
    header.sequence = ++m_sequenceNumber;

    const std::vector<uint8_t> frame = rnp::serialize(header, payload.data());

    for (const auto &[endpoint, clientInfo] : m_clients)
    {
        if (clientInfo.connected)
        {
            m_socket.async_send_to(asio::buffer(frame), endpoint,
                                   [this](const asio::error_code &error, std::size_t bytesTransferred)
                                   { handleSend(error, bytesTransferred); });
        }
    }
}

void srv::AsioServer::setPacketHandler(rnp::PacketType type, PacketHandler handler)
{
    m_packetHandlers[type] = handler;
}
