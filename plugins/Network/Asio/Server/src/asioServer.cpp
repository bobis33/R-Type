#include <cstring>
#include <iostream>

#include "AsioServer/AsioServer.hpp"

using asio::ip::udp;

srv::AsioServer::AsioServer() : m_socket(m_ioContext), m_recvBuffer() {}

void srv::AsioServer::init(const std::string &host, const uint16_t port)
{
    const asio::ip::address addr = asio::ip::make_address(host);
    const udp::endpoint ep(addr, port);

    m_socket.open(ep.protocol());
    m_socket.set_option(asio::socket_base::reuse_address(true));
    m_socket.bind(ep);
}

void srv::AsioServer::start()
{
    m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(
        asio::make_work_guard(m_ioContext));

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
    if (m_workGuard)
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

srv::AsioServer::~AsioServer() { stop(); }

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
        if (header.length > 0 && data.size() >= 16 + header.length)
        {
            payload.assign(data.begin() + 16, data.begin() + 16 + header.length);
        }

        // Vérifier la session ID (sauf pour CONNECT)
        if (static_cast<rnp::PacketType>(header.type) != rnp::PacketType::CONNECT)
        {
            auto it = m_clients.find(sender);
            if (it != m_clients.end() && it->second.sessionId != header.sessionId)
            {
                sendError(sender, rnp::ErrorCode::UNAUTHORIZED_SESSION, "Invalid session ID");
                return;
            }
        }

        // Gérer les flags de fiabilité
        if (header.flags & static_cast<std::uint16_t>(rnp::PacketFlags::RELIABLE))
        {
            handleReliablePacket(sender, header);
        }
        if (header.flags & static_cast<std::uint16_t>(rnp::PacketFlags::ACK_REQ))
        {
            sendAck(sender, header.sequence, 0);
        }

        switch (static_cast<rnp::PacketType>(header.type))
        {
            case rnp::PacketType::CONNECT:
            {
                if (payload.size() >= 5)
                {
                    std::uint8_t nameLen = payload[0];
                    if (payload.size() >= 1 + nameLen + 4)
                    {
                        std::string playerName(payload.begin() + 1, payload.begin() + 1 + nameLen);
                        std::uint32_t clientCaps = (static_cast<std::uint32_t>(payload[1 + nameLen]) << 24) |
                                                   (static_cast<std::uint32_t>(payload[1 + nameLen + 1]) << 16) |
                                                   (static_cast<std::uint32_t>(payload[1 + nameLen + 2]) << 8) |
                                                   static_cast<std::uint32_t>(payload[1 + nameLen + 3]);

                        std::uint32_t sessionId = m_nextSessionId++;
                        addClient(sender, playerName, clientCaps, sessionId);
                        sendConnectAccept(sender, sessionId);
                        std::cout << "[AsioServer] Client connecté: " << playerName << " ("
                                  << sender.address().to_string() << ":" << sender.port()
                                  << ") - Session: " << sessionId << "\n";
                    }
                }
                break;
            }
            case rnp::PacketType::DISCONNECT:
            {
                if (payload.size() >= 2)
                {
                    std::uint16_t reason =
                        (static_cast<std::uint16_t>(payload[0]) << 8) | static_cast<std::uint16_t>(payload[1]);
                    std::cout << "[AsioServer] Client déconnecté: " << sender.address().to_string() << ":"
                              << sender.port() << " - Reason: " << reason << "\n";
                }
                removeClient(sender);
                break;
            }
            case rnp::PacketType::ACK:
            {
                processAck(sender, payload);
                break;
            }
            case rnp::PacketType::ENTITY_EVENT:
            {
                try
                {
                    const std::vector<rnp::EventRecord> events = rnp::deserializeEvents(payload.data(), payload.size());

                    // Broadcast les events aux autres clients
                    for (const auto &[endpoint, clientInfo] : m_clients)
                    {
                        if (endpoint != sender && clientInfo.connected)
                        {
                            sendEntityEvent(endpoint, 0, events);
                        }
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "[AsioServer] Erreur parsing ENTITY_EVENT: " << e.what() << "\n";
                }
                break;
            }
            case rnp::PacketType::PLAYER_INPUT:
            {
                // Support legacy PLAYER_INPUT
                if (payload.size() >= 2)
                {
                    const std::uint8_t direction = payload[0];
                    const std::uint8_t shooting = payload[1];

                    rnp::EventRecord ev;
                    ev.type = rnp::EventType::INPUT;
                    ev.entityId = getPlayerId(sender);
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
                if (payload.size() >= 8)
                {
                    std::uint32_t nonce = (static_cast<std::uint32_t>(payload[0]) << 24) |
                                          (static_cast<std::uint32_t>(payload[1]) << 16) |
                                          (static_cast<std::uint32_t>(payload[2]) << 8) |
                                          static_cast<std::uint32_t>(payload[3]);
                    std::uint32_t sendTime = (static_cast<std::uint32_t>(payload[4]) << 24) |
                                             (static_cast<std::uint32_t>(payload[5]) << 16) |
                                             (static_cast<std::uint32_t>(payload[6]) << 8) |
                                             static_cast<std::uint32_t>(payload[7]);
                    sendPong(sender, nonce, sendTime);
                }
                else
                {
                    sendPong(sender);
                }
                break;
            }
            default:
                break;
        }

        auto it = m_packetHandlers.find(static_cast<rnp::PacketType>(header.type));
        if (it != m_packetHandlers.end())
        {
            it->second(sender, header, payload);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AsioServer] Erreur de traitement du paquet: " << e.what() << "\n";
        sendError(sender, rnp::ErrorCode::INVALID_PAYLOAD, "Erreur de traitement du paquet");
    }
}

void srv::AsioServer::addClient(const asio::ip::udp::endpoint &endpoint, const std::string &playerName,
                                std::uint32_t clientCaps, std::uint32_t sessionId)
{
    ClientInfo info;
    info.endpoint = endpoint;
    info.playerName = playerName;
    info.lastSequence = 0;
    info.connected = true;
    info.playerId = m_nextPlayerId++;
    info.sessionId = sessionId;
    info.clientCaps = clientCaps;
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

std::uint32_t srv::AsioServer::getSessionId(const asio::ip::udp::endpoint &endpoint) const
{
    auto it = m_clients.find(endpoint);
    if (it != m_clients.end())
    {
        return it->second.sessionId;
    }
    return 0;
}

void srv::AsioServer::sendConnectAccept(const asio::ip::udp::endpoint &client, std::uint32_t sessionId)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::CONNECT_ACCEPT);

    // Payload: session_id(4, BE) | tick_rate_hz(2, BE) | mtu_payload_bytes(2, BE) | server_caps(4, BE)
    std::vector<uint8_t> payload;

    // session_id (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((sessionId >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((sessionId >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((sessionId >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(sessionId & 0xFF));

    // tick_rate_hz (2 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((m_tickRateHz >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(m_tickRateHz & 0xFF));

    // mtu_payload_bytes (2 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((m_mtuPayloadBytes >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(m_mtuPayloadBytes & 0xFF));

    // server_caps (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((m_serverCaps >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((m_serverCaps >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((m_serverCaps >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(m_serverCaps & 0xFF));

    header.length = payload.size();
    header.flags =
        static_cast<std::uint16_t>(rnp::PacketFlags::RELIABLE) | static_cast<std::uint16_t>(rnp::PacketFlags::ACK_REQ);
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = sessionId;

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendAck(const asio::ip::udp::endpoint &client, std::uint32_t cumulative, std::uint32_t ackBits)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::ACK);

    // Payload: cumulative_ack(4, BE) | ack_bits(4, BE)
    std::vector<uint8_t> payload;

    // cumulative_ack (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((cumulative >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((cumulative >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((cumulative >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(cumulative & 0xFF));

    // ack_bits (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((ackBits >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((ackBits >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((ackBits >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(ackBits & 0xFF));

    header.length = payload.size();
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendWorldState(const asio::ip::udp::endpoint &client, const std::vector<uint8_t> &worldData)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::WORLD_STATE);
    header.length = worldData.size();
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, worldData.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendWorldState(const asio::ip::udp::endpoint &client, std::uint32_t serverTick,
                                     const std::vector<rnp::EntityState> &entities)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::WORLD_STATE);

    // Payload: server_tick(4, BE) | entity_count(2, BE) | entities...
    std::vector<uint8_t> payload;

    // server_tick (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((serverTick >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((serverTick >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((serverTick >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(serverTick & 0xFF));

    // entity_count (2 bytes, big endian)
    std::uint16_t entityCount = static_cast<std::uint16_t>(entities.size());
    payload.push_back(static_cast<uint8_t>((entityCount >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(entityCount & 0xFF));

    // Pour chaque entité: id(4) | type(2) | x(4) | y(4) | vx(4) | vy(4) | state_flags(1)
    for (const auto &entity : entities)
    {
        // id (4 bytes, big endian)
        payload.push_back(static_cast<uint8_t>((entity.id >> 24) & 0xFF));
        payload.push_back(static_cast<uint8_t>((entity.id >> 16) & 0xFF));
        payload.push_back(static_cast<uint8_t>((entity.id >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(entity.id & 0xFF));

        // type (2 bytes, big endian)
        payload.push_back(static_cast<uint8_t>((entity.type >> 8) & 0xFF));
        payload.push_back(static_cast<uint8_t>(entity.type & 0xFF));

        // x, y, vx, vy (floats en big endian)
        const uint8_t *xBytes = reinterpret_cast<const uint8_t *>(&entity.x);
        const uint8_t *yBytes = reinterpret_cast<const uint8_t *>(&entity.y);
        const uint8_t *vxBytes = reinterpret_cast<const uint8_t *>(&entity.vx);
        const uint8_t *vyBytes = reinterpret_cast<const uint8_t *>(&entity.vy);

        // Note: Assuming little endian system, reverse for big endian network order
        for (int i = 3; i >= 0; --i)
            payload.push_back(xBytes[i]);
        for (int i = 3; i >= 0; --i)
            payload.push_back(yBytes[i]);
        for (int i = 3; i >= 0; --i)
            payload.push_back(vxBytes[i]);
        for (int i = 3; i >= 0; --i)
            payload.push_back(vyBytes[i]);

        // state_flags (1 byte)
        payload.push_back(entity.stateFlags);
    }

    header.length = payload.size();
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendEvents(const asio::ip::udp::endpoint &client, const std::vector<rnp::EventRecord> &events)
{
    const std::vector<uint8_t> eventsPayload = rnp::serializeEvents(events);

    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::ENTITY_EVENT);
    header.length = static_cast<std::uint16_t>(eventsPayload.size());
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, eventsPayload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendEntityEvent(const asio::ip::udp::endpoint &client, std::uint32_t serverTick,
                                      const std::vector<rnp::EventRecord> &events)
{
    const std::vector<uint8_t> eventsPayload = rnp::serializeEvents(events);

    // Payload: server_tick(4, BE) | event_count(2, BE) | events...
    std::vector<uint8_t> payload;

    // server_tick (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((serverTick >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((serverTick >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((serverTick >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(serverTick & 0xFF));

    // event_count (2 bytes, big endian)
    std::uint16_t eventCount = static_cast<std::uint16_t>(events.size());
    payload.push_back(static_cast<uint8_t>((eventCount >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(eventCount & 0xFF));

    // Events serialized
    payload.insert(payload.end(), eventsPayload.begin(), eventsPayload.end());

    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::ENTITY_EVENT);
    header.length = static_cast<std::uint16_t>(payload.size());
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendPong(const asio::ip::udp::endpoint &client)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::PONG);
    header.length = 0;
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header);

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendPong(const asio::ip::udp::endpoint &client, std::uint32_t nonce, std::uint32_t sendTimeMs)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::PONG);

    // Payload: nonce(4, BE) | send_time_ms(4, BE)
    std::vector<uint8_t> payload;

    // nonce (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((nonce >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((nonce >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((nonce >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(nonce & 0xFF));

    // send_time_ms (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((sendTimeMs >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((sendTimeMs >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((sendTimeMs >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(sendTimeMs & 0xFF));

    header.length = payload.size();
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), client,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void srv::AsioServer::sendError(const asio::ip::udp::endpoint &client, const std::string &errorMessage)
{
    sendError(client, rnp::ErrorCode::INTERNAL_ERROR, errorMessage);
}

void srv::AsioServer::sendError(const asio::ip::udp::endpoint &client, rnp::ErrorCode errorCode,
                                const std::string &errorMessage)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::PACKET_ERROR);

    // Payload: error_code(2, BE) | msg_len(2, BE) | message
    std::vector<uint8_t> payload;
    std::uint16_t code = static_cast<std::uint16_t>(errorCode);
    payload.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(code & 0xFF));

    std::uint16_t msgLen = static_cast<std::uint16_t>(errorMessage.size());
    payload.push_back(static_cast<uint8_t>((msgLen >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(msgLen & 0xFF));

    payload.insert(payload.end(), errorMessage.begin(), errorMessage.end());

    header.length = payload.size();
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = getSessionId(client);

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

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
    header.type = static_cast<std::uint8_t>(rnp::PacketType::ENTITY_EVENT);
    header.length = static_cast<std::uint16_t>(payload.size());
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;

    for (const auto &[endpoint, clientInfo] : m_clients)
    {
        if (clientInfo.connected)
        {
            header.sessionId = clientInfo.sessionId;
            const std::vector<uint8_t> frame = rnp::serialize(header, payload.data());

            m_socket.async_send_to(asio::buffer(frame), endpoint,
                                   [this](const asio::error_code &error, std::size_t bytesTransferred)
                                   { handleSend(error, bytesTransferred); });
        }
    }
}

void srv::AsioServer::broadcastEntityEvents(std::uint32_t serverTick, const std::vector<rnp::EventRecord> &events)
{
    const std::vector<uint8_t> eventsPayload = rnp::serializeEvents(events);

    // Payload: server_tick(4, BE) | event_count(2, BE) | events...
    std::vector<uint8_t> payload;

    // server_tick (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((serverTick >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((serverTick >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((serverTick >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(serverTick & 0xFF));

    // event_count (2 bytes, big endian)
    std::uint16_t eventCount = static_cast<std::uint16_t>(events.size());
    payload.push_back(static_cast<uint8_t>((eventCount >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(eventCount & 0xFF));

    // Events serialized
    payload.insert(payload.end(), eventsPayload.begin(), eventsPayload.end());

    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::ENTITY_EVENT);
    header.length = static_cast<std::uint16_t>(payload.size());
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;

    for (const auto &[endpoint, clientInfo] : m_clients)
    {
        if (clientInfo.connected)
        {
            header.sessionId = clientInfo.sessionId;
            const std::vector<uint8_t> frame = rnp::serialize(header, payload.data());

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

void srv::AsioServer::handleReliablePacket(const asio::ip::udp::endpoint &sender, const rnp::PacketHeader &header)
{
    // Store for potential retransmission (simplified implementation)
    // In production, you'd want more sophisticated tracking
    auto it = m_clients.find(sender);
    if (it != m_clients.end())
    {
        it->second.lastSequence = header.sequence;
    }
}

void srv::AsioServer::processAck(const asio::ip::udp::endpoint &sender, const std::vector<uint8_t> &payload)
{
    if (payload.size() < 8)
    {
        return;
    }

    // cumulative_ack (4 bytes, big endian)
    std::uint32_t cumulative = (static_cast<std::uint32_t>(payload[0]) << 24) |
                               (static_cast<std::uint32_t>(payload[1]) << 16) |
                               (static_cast<std::uint32_t>(payload[2]) << 8) | static_cast<std::uint32_t>(payload[3]);

    // ack_bits (4 bytes, big endian)
    std::uint32_t ackBits = (static_cast<std::uint32_t>(payload[4]) << 24) |
                            (static_cast<std::uint32_t>(payload[5]) << 16) |
                            (static_cast<std::uint32_t>(payload[6]) << 8) | static_cast<std::uint32_t>(payload[7]);

    // Remove acknowledged packets from pending reliable
    m_pendingReliable.erase(cumulative);

    // Process ack bits for selective acknowledgment
    for (int i = 0; i < 32; ++i)
    {
        if (ackBits & (1 << i))
        {
            m_pendingReliable.erase(cumulative - i - 1);
        }
    }

    m_clientLastAck[sender] = cumulative;
}

void srv::AsioServer::retransmitReliable()
{
    // Simplified retransmission logic
    // In production, track timestamps and implement exponential backoff
    for (const auto &[seq, data] : m_pendingReliable)
    {
        // Retransmit to all clients (would need per-client tracking in production)
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
}
