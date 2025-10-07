#include <cstring>
#include <iostream>

#include <asio.hpp>

#include "AsioClient/AsioClient.hpp"

using asio::ip::udp;

eng::AsioClient::AsioClient() : m_socket(m_ioContext) {}

void eng::AsioClient::connect(const std::string &host, uint16_t port)
{
    try
    {
        const asio::ip::address addr = asio::ip::make_address(host);
        m_serverEndpoint = udp::endpoint(addr, port);

        m_socket.open(udp::v4());

        m_workGuard = std::make_unique<asio::executor_work_guard<asio::io_context::executor_type>>(asio::make_work_guard(m_ioContext));
        m_ioThread = std::thread(
            [this]()
            {
                try
                {
                    m_ioContext.run();
                }
                catch (const std::exception &e)
                {
                    std::cerr << "[AsioClient] IO context exception: " << e.what() << "\n";
                }
            });

        startReceive();
        m_connected = true;

        std::cout << "[AsioClient] Connecté au serveur " << host << ":" << port << "\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AsioClient] Erreur de connexion: " << e.what() << "\n";
    }
}

void eng::AsioClient::disconnect()
{
    if (m_connected)
    {
        sendDisconnect();

        if (m_workGuard)
        {
            asio::post(m_ioContext,
                       [this]()
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

        m_connected = false;
        std::cout << "[AsioClient] Déconnecté du serveur\n";
    }
}

void eng::AsioClient::sendConnect(const std::string &playerName)
{
    sendConnectWithCaps(playerName, 0);
}

void eng::AsioClient::sendConnectWithCaps(const std::string &playerName, std::uint32_t clientCaps)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::CONNECT);
    
    // Payload: name_len(1) | player_name[name_len] | client_caps(4, BE)
    std::vector<uint8_t> payload;
    std::uint8_t nameLen = std::min(static_cast<std::uint8_t>(playerName.size()),
                                   static_cast<std::uint8_t>(31));
    payload.push_back(nameLen);
    payload.insert(payload.end(), playerName.begin(), playerName.begin() + nameLen);
    
    // client_caps (4 bytes, big endian)
    payload.push_back(static_cast<uint8_t>((clientCaps >> 24) & 0xFF));
    payload.push_back(static_cast<uint8_t>((clientCaps >> 16) & 0xFF));
    payload.push_back(static_cast<uint8_t>((clientCaps >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(clientCaps & 0xFF));
    
    header.length = payload.size();
    header.flags = static_cast<std::uint16_t>(rnp::PacketFlags::RELIABLE) |
                  static_cast<std::uint16_t>(rnp::PacketFlags::ACK_REQ);
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = 0;  // Pas encore de session

    m_clientCaps = clientCaps;
    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                          [this](const asio::error_code &error, std::size_t bytesTransferred)
                          { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendDisconnect()
{
    sendDisconnect(rnp::DisconnectReason::CLIENT_REQUEST);
}

void eng::AsioClient::sendDisconnect(rnp::DisconnectReason reason)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::DISCONNECT);
    
    // Payload: reason_code(2, BE)
    std::vector<uint8_t> payload;
    std::uint16_t reasonCode = static_cast<std::uint16_t>(reason);
    payload.push_back(static_cast<uint8_t>((reasonCode >> 8) & 0xFF));
    payload.push_back(static_cast<uint8_t>(reasonCode & 0xFF));
    
    header.length = payload.size();
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = m_sessionId;

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendPlayerInput(uint8_t direction, uint8_t shooting)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::PLAYER_INPUT);
    header.length = 2;
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = m_sessionId;

    uint8_t payload[2] = {direction, shooting};
    std::vector<uint8_t> buffer = rnp::serialize(header, payload);

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendPlayerInputAsEvent(std::uint16_t playerId, uint8_t direction, uint8_t shooting,
                                             uint32_t clientTimeMs)
{
    rnp::EventRecord ev;
    ev.type = rnp::EventType::INPUT;
    ev.entityId = playerId;
    
    // Data: buttons(2, BE) | direction(1) | shooting(1) | client_time_ms(4, BE)
    std::uint16_t buttons = 0;  // TODO: map from direction/shooting to buttons
    ev.data.push_back(static_cast<uint8_t>((buttons >> 8) & 0xFF));
    ev.data.push_back(static_cast<uint8_t>(buttons & 0xFF));
    ev.data.push_back(direction);
    ev.data.push_back(shooting);
    ev.data.push_back(static_cast<uint8_t>((clientTimeMs >> 24) & 0xFF));
    ev.data.push_back(static_cast<uint8_t>((clientTimeMs >> 16) & 0xFF));
    ev.data.push_back(static_cast<uint8_t>((clientTimeMs >> 8) & 0xFF));
    ev.data.push_back(static_cast<uint8_t>(clientTimeMs & 0xFF));
    
    std::vector<rnp::EventRecord> events;
    events.push_back(ev);
    
    const std::vector<uint8_t> eventsPayload = rnp::serializeEvents(events);
    
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::ENTITY_EVENT);
    header.length = static_cast<std::uint16_t>(eventsPayload.size());
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = m_sessionId;

    std::vector<uint8_t> buffer = rnp::serialize(header, eventsPayload.data());

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendPing()
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::PING);
    header.length = 0;
    header.flags = 0;
    header.reserved = 0;
    header.sequence = ++m_sequenceNumber;
    header.sessionId = m_sessionId;

    std::vector<uint8_t> buffer = rnp::serialize(header);

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendPing(std::uint32_t nonce, std::uint32_t sendTimeMs)
{
    rnp::PacketHeader header;
    header.type = static_cast<std::uint8_t>(rnp::PacketType::PING);
    
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
    header.sessionId = m_sessionId;

    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendAck(std::uint32_t cumulative, std::uint32_t ackBits)
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
    header.sessionId = m_sessionId;

    m_lastAckSent = cumulative;
    std::vector<uint8_t> buffer = rnp::serialize(header, payload.data());

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::setPacketHandler(rnp::PacketType type, PacketHandler handler)
{
    m_packetHandlers[type] = handler;
}

void eng::AsioClient::setEventsHandler(std::function<void(const std::vector<rnp::EventRecord> &)> handler)
{
    m_eventsHandler = std::move(handler);
}

void eng::AsioClient::handleConnectAccept(const std::vector<uint8_t> &payload)
{
    if (payload.size() < 12)
    {
        std::cerr << "[AsioClient] Invalid CONNECT_ACCEPT payload\n";
        return;
    }

    // session_id(4, BE)
    m_sessionId = (static_cast<std::uint32_t>(payload[0]) << 24) |
                  (static_cast<std::uint32_t>(payload[1]) << 16) |
                  (static_cast<std::uint32_t>(payload[2]) << 8) |
                  static_cast<std::uint32_t>(payload[3]);

    // tick_rate_hz(2, BE)
    m_serverTickRate = (static_cast<std::uint16_t>(payload[4]) << 8) |
                       static_cast<std::uint16_t>(payload[5]);

    // mtu_payload_bytes(2, BE)
    m_serverMtu = (static_cast<std::uint16_t>(payload[6]) << 8) |
                  static_cast<std::uint16_t>(payload[7]);

    // server_caps(4, BE)
    m_serverCaps = (static_cast<std::uint32_t>(payload[8]) << 24) |
                   (static_cast<std::uint32_t>(payload[9]) << 16) |
                   (static_cast<std::uint32_t>(payload[10]) << 8) |
                   static_cast<std::uint32_t>(payload[11]);

    std::cout << "[AsioClient] Connection accepted - Session ID: " << m_sessionId
              << ", Tick Rate: " << m_serverTickRate << " Hz"
              << ", MTU: " << m_serverMtu << " bytes\n";
}

void eng::AsioClient::handleReliablePacket(const rnp::PacketHeader &header)
{
    // Track reliable packets (simplified implementation)
    // In production, implement proper acknowledgment tracking
}

void eng::AsioClient::processAck(const std::vector<uint8_t> &payload)
{
    if (payload.size() < 8)
    {
        return;
    }

    // cumulative_ack (4 bytes, big endian)
    std::uint32_t cumulative = (static_cast<std::uint32_t>(payload[0]) << 24) |
                               (static_cast<std::uint32_t>(payload[1]) << 16) |
                               (static_cast<std::uint32_t>(payload[2]) << 8) |
                               static_cast<std::uint32_t>(payload[3]);

    // ack_bits (4 bytes, big endian)
    std::uint32_t ackBits = (static_cast<std::uint32_t>(payload[4]) << 24) |
                            (static_cast<std::uint32_t>(payload[5]) << 16) |
                            (static_cast<std::uint32_t>(payload[6]) << 8) |
                            static_cast<std::uint32_t>(payload[7]);

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
}

void eng::AsioClient::processWorldState(const std::vector<uint8_t> &payload)
{
    if (payload.size() < 6)
    {
        return;
    }

    // server_tick (4 bytes, big endian)
    std::uint32_t serverTick = (static_cast<std::uint32_t>(payload[0]) << 24) |
                               (static_cast<std::uint32_t>(payload[1]) << 16) |
                               (static_cast<std::uint32_t>(payload[2]) << 8) |
                               static_cast<std::uint32_t>(payload[3]);

    // entity_count (2 bytes, big endian)
    std::uint16_t entityCount = (static_cast<std::uint16_t>(payload[4]) << 8) |
                                static_cast<std::uint16_t>(payload[5]);

    std::cout << "[AsioClient] World state received - Tick: " << serverTick
              << ", Entities: " << entityCount << "\n";

    // TODO: Parse entities and call appropriate handler
}

void eng::AsioClient::processEntityEvent(const std::vector<uint8_t> &payload)
{
    try
    {
        if (payload.size() < 6)
        {
            // Legacy format without server_tick
            const std::vector<rnp::EventRecord> events = rnp::deserializeEvents(payload.data(), payload.size());
            if (m_eventsHandler)
            {
                m_eventsHandler(events);
            }
            return;
        }

        // New format with server_tick
        // server_tick (4 bytes, big endian)
        std::uint32_t serverTick = (static_cast<std::uint32_t>(payload[0]) << 24) |
                                   (static_cast<std::uint32_t>(payload[1]) << 16) |
                                   (static_cast<std::uint32_t>(payload[2]) << 8) |
                                   static_cast<std::uint32_t>(payload[3]);

        // event_count (2 bytes, big endian)
        std::uint16_t eventCount = (static_cast<std::uint16_t>(payload[4]) << 8) |
                                   static_cast<std::uint16_t>(payload[5]);

        // Events serialized
        const std::vector<rnp::EventRecord> events = rnp::deserializeEvents(payload.data() + 6, payload.size() - 6);

        std::cout << "[AsioClient] Entity events received - Tick: " << serverTick
                  << ", Events: " << eventCount << "\n";

        if (m_eventsHandler)
        {
            m_eventsHandler(events);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AsioClient] Erreur de parsing ENTITY_EVENT: " << e.what() << "\n";
    }
}

void eng::AsioClient::retransmitReliable()
{
    // Simplified retransmission logic
    // In production, track timestamps and implement exponential backoff
    for (const auto &[seq, data] : m_pendingReliable)
    {
        m_socket.async_send_to(asio::buffer(data), m_serverEndpoint,
                              [this](const asio::error_code &error, std::size_t bytesTransferred)
                              { handleSend(error, bytesTransferred); });
    }
}

void eng::AsioClient::startReceive()
{
    m_socket.async_receive_from(asio::buffer(m_recvBuffer), m_serverEndpoint,
                                [this](const asio::error_code &error, std::size_t bytesTransferred)
                                { handleReceive(error, bytesTransferred); });
}

void eng::AsioClient::handleReceive(const asio::error_code &error, std::size_t bytesTransferred)
{
    if (!error)
    {
        std::vector<uint8_t> data(m_recvBuffer.begin(), m_recvBuffer.begin() + bytesTransferred);
        processPacket(data);
        startReceive();
    }
    else if (error != asio::error::operation_aborted)
    {
        std::cerr << "[AsioClient] Erreur de réception: " << error.message() << "\n";
        startReceive();
    }
}

void eng::AsioClient::handleSend(const asio::error_code &error, std::size_t bytesTransferred)
{
    if (error)
    {
        std::cerr << "[AsioClient] Erreur d'envoi: " << error.message() << "\n";
    }
}

void eng::AsioClient::processPacket(const std::vector<uint8_t> &data)
{
    try
    {
        rnp::PacketHeader header = rnp::deserializeHeader(data.data(), data.size());

        std::vector<uint8_t> payload;
        if (header.length > 0 && data.size() >= 16 + header.length)
        {
            payload.assign(data.begin() + 16, data.begin() + 16 + header.length);
        }

        // Vérifier la session ID (sauf pour CONNECT_ACCEPT)
        if (static_cast<rnp::PacketType>(header.type) != rnp::PacketType::CONNECT_ACCEPT &&
            m_sessionId != 0 && header.sessionId != m_sessionId)
        {
            std::cerr << "[AsioClient] Invalid session ID in packet\n";
            return;
        }

        // Gérer les flags de fiabilité
        if (header.flags & static_cast<std::uint16_t>(rnp::PacketFlags::RELIABLE))
        {
            handleReliablePacket(header);
        }
        if (header.flags & static_cast<std::uint16_t>(rnp::PacketFlags::ACK_REQ))
        {
            sendAck(header.sequence, 0);
        }

        switch (static_cast<rnp::PacketType>(header.type))
        {
            case rnp::PacketType::CONNECT_ACCEPT:
            {
                handleConnectAccept(payload);
                break;
            }
            case rnp::PacketType::WORLD_STATE:
            {
                processWorldState(payload);
                break;
            }
            case rnp::PacketType::ENTITY_EVENT:
            {
                processEntityEvent(payload);
                break;
            }
            case rnp::PacketType::ACK:
            {
                processAck(payload);
                break;
            }
            case rnp::PacketType::ERROR:
            {
                if (payload.size() >= 4)
                {
                    std::uint16_t errorCode = (static_cast<std::uint16_t>(payload[0]) << 8) |
                                             static_cast<std::uint16_t>(payload[1]);
                    std::uint16_t msgLen = (static_cast<std::uint16_t>(payload[2]) << 8) |
                                          static_cast<std::uint16_t>(payload[3]);
                    if (payload.size() >= 4 + msgLen)
                    {
                        std::string errorMsg(payload.begin() + 4, payload.begin() + 4 + msgLen);
                        std::cerr << "[AsioClient] Error " << errorCode << ": " << errorMsg << "\n";
                    }
                }
                break;
            }
            case rnp::PacketType::PONG:
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
                    std::cout << "[AsioClient] PONG received - nonce: " << nonce << ", time: " << sendTime << "\n";
                }
                break;
            }
            default:
                break;
        }

        // Appeler les handlers personnalisés
        auto it = m_packetHandlers.find(static_cast<rnp::PacketType>(header.type));
        if (it != m_packetHandlers.end())
        {
            it->second(header, payload);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AsioClient] Erreur de traitement du paquet: " << e.what() << "\n";
    }
}
