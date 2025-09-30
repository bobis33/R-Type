#include <cstring>
#include <iostream>

#include <asio.hpp>

#include "AsioClient/AsioClient.hpp"

using asio::ip::udp;

eng::AsioClient::AsioClient() : m_socket(m_ioContext) {}

eng::AsioClient::~AsioClient() { disconnect(); }

void eng::AsioClient::connect(const std::string &host, uint16_t port)
{
    try
    {
        const asio::ip::address addr = asio::ip::make_address(host);
        m_serverEndpoint = udp::endpoint(addr, port);

        m_socket.open(udp::v4());

        m_workGuard.emplace(asio::make_work_guard(m_ioContext));
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

        if (m_workGuard.has_value())
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
    rnp::PacketConnect packet;
    packet.header.version = rnp::PROTOCOL_VERSION;
    packet.header.type = rnp::PacketType::CONNECT;
    packet.header.length = sizeof(packet.player_name);
    packet.header.sequence = ++m_sequenceNumber;

    std::strncpy(packet.player_name, playerName.c_str(), sizeof(packet.player_name) - 1);
    packet.player_name[sizeof(packet.player_name) - 1] = '\0';

    std::vector<uint8_t> buffer = rnp::serialize(packet.header, reinterpret_cast<const uint8_t *>(packet.player_name));

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendDisconnect()
{
    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::DISCONNECT;
    header.length = 0;
    header.sequence = ++m_sequenceNumber;

    std::vector<uint8_t> buffer = rnp::serialize(header);

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendPlayerInput(uint8_t direction, uint8_t shooting)
{
    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::PLAYER_INPUT;
    header.length = 2;
    header.sequence = ++m_sequenceNumber;

    uint8_t payload[2] = {direction, shooting};
    std::vector<uint8_t> buffer = rnp::serialize(header, payload);

    m_socket.async_send_to(asio::buffer(buffer), m_serverEndpoint,
                           [this](const asio::error_code &error, std::size_t bytesTransferred)
                           { handleSend(error, bytesTransferred); });
}

void eng::AsioClient::sendPing()
{
    rnp::PacketHeader header;
    header.version = rnp::PROTOCOL_VERSION;
    header.type = rnp::PacketType::PING;
    header.length = 0;
    header.sequence = ++m_sequenceNumber;

    std::vector<uint8_t> buffer = rnp::serialize(header);

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
        if (header.length > 0 && data.size() >= sizeof(rnp::PacketHeader) + header.length)
        {
            payload.assign(data.begin() + sizeof(rnp::PacketHeader),
                           data.begin() + sizeof(rnp::PacketHeader) + header.length);
        }

        if (header.type == rnp::PacketType::EVENTS)
        {
            try
            {
                const std::vector<rnp::EventRecord> events = rnp::deserializeEvents(payload.data(), payload.size());
                if (m_eventsHandler)
                {
                    m_eventsHandler(events);
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[AsioClient] Erreur de parsing EVENTS: " << e.what() << "\n";
            }
            return;
        }

        auto it = m_packetHandlers.find(header.type);
        if (it != m_packetHandlers.end())
        {
            it->second(header, payload);
        }
        else
        {
            std::cout << "[AsioClient] Paquet reçu de type " << static_cast<int>(header.type) << " sans gestionnaire\n";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AsioClient] Erreur de traitement du paquet: " << e.what() << "\n";
    }
}
