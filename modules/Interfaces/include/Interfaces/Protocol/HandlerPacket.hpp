///
/// @file HandlerPacket.hpp
/// @brief Network packet handler for RNP protocol
/// @namespace rnp
///

#pragma once

#include "Protocol.hpp"
#include "Serializer.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace rnp
{

    ///
    /// @brief Packet processing result
    ///
    enum class HandlerResult
    {
        SUCCESS,
        INVALID_PACKET,
        UNSUPPORTED_TYPE,
        PROCESSING_ERROR,
        SESSION_INVALID,
        RATE_LIMITED
    };

    ///
    /// @brief Context information for packet processing
    ///
    struct PacketContext
    {
            std::uint32_t sessionId;
            std::chrono::steady_clock::time_point receiveTime;
            std::string senderAddress;
            std::uint16_t senderPort;
            bool isReliable;
            bool isCompressed;
    };

    ///
    /// @brief Forward declarations for callback types
    ///
    class HandlerPacket;

    using ConnectHandler = std::function<HandlerResult(const PacketConnect &, const PacketContext &)>;
    using ConnectAcceptHandler = std::function<HandlerResult(const PacketConnectAccept &, const PacketContext &)>;
    using DisconnectHandler = std::function<HandlerResult(const PacketDisconnect &, const PacketContext &)>;
    using WorldStateHandler = std::function<HandlerResult(const PacketWorldState &, const PacketContext &)>;
    using PingHandler = std::function<HandlerResult(const PacketPingPong &, const PacketContext &)>;
    using PongHandler = std::function<HandlerResult(const PacketPingPong &, const PacketContext &)>;
    using ErrorHandler = std::function<HandlerResult(const PacketError &, const PacketContext &)>;
    using EntityEventHandler = std::function<HandlerResult(const std::vector<EventRecord> &, const PacketContext &)>;
    using AckHandler = std::function<HandlerResult(std::uint32_t sequenceId, const PacketContext &)>;

    ///
    /// @brief Statistics for packet handling
    ///
    struct HandlerStats
    {
            std::uint64_t totalPacketsReceived = 0;
            std::uint64_t totalPacketsProcessed = 0;
            std::uint64_t totalPacketsDropped = 0;
            std::uint64_t totalBytesReceived = 0;
            std::unordered_map<PacketType, std::uint64_t> packetTypeCount;
            std::unordered_map<HandlerResult, std::uint64_t> resultCount;
    };

    ///
    /// @brief Main packet handler class
    ///
    class HandlerPacket
    {
        private:
            // Handler callbacks
            ConnectHandler connectHandler_;
            ConnectAcceptHandler connectAcceptHandler_;
            DisconnectHandler disconnectHandler_;
            WorldStateHandler worldStateHandler_;
            PingHandler pingHandler_;
            PongHandler pongHandler_;
            ErrorHandler errorHandler_;
            EntityEventHandler entityEventHandler_;
            AckHandler ackHandler_;

            // Statistics
            HandlerStats stats_;

            // Rate limiting (simple token bucket per session)
            std::unordered_map<std::uint32_t, std::chrono::steady_clock::time_point> lastPacketTime_;
            std::chrono::milliseconds rateLimitInterval_{10}; // 10ms minimum between packets

            ///
            /// @brief Check rate limiting for a session
            /// @param sessionId Session to check
            /// @param currentTime Current time
            /// @return True if packet should be rate limited
            ///
            bool isRateLimited(std::uint32_t sessionId, const std::chrono::steady_clock::time_point &currentTime)
            {
                auto it = lastPacketTime_.find(sessionId);
                if (it == lastPacketTime_.end())
                {
                    lastPacketTime_[sessionId] = currentTime;
                    return false;
                }

                auto timeSinceLastPacket = currentTime - it->second;
                if (timeSinceLastPacket < rateLimitInterval_)
                {
                    return true;
                }

                it->second = currentTime;
                return false;
            }

            ///
            /// @brief Update statistics
            /// @param packetType Type of packet processed
            /// @param result Processing result
            /// @param bytesReceived Number of bytes in the packet
            ///
            void updateStats(PacketType packetType, HandlerResult result, std::size_t bytesReceived)
            {
                stats_.totalPacketsReceived++;
                stats_.totalBytesReceived += bytesReceived;
                stats_.packetTypeCount[packetType]++;
                stats_.resultCount[result]++;

                if (result == HandlerResult::SUCCESS)
                {
                    stats_.totalPacketsProcessed++;
                }
                else
                {
                    stats_.totalPacketsDropped++;
                }
            }

            ///
            /// @brief Parse and handle ENTITY_EVENT packet
            /// @param serializer Serializer containing the packet data
            /// @param payloadSize Size of the payload
            /// @return Vector of event records
            ///
            std::vector<EventRecord> parseEntityEvents(Serializer &serializer, std::size_t payloadSize)
            {
                std::vector<EventRecord> events;
                std::size_t bytesRead = 0;

                while (bytesRead < payloadSize)
                {
                    EventRecord event;
                    event.type = static_cast<EventType>(serializer.readByte());
                    event.entityId = serializer.readUInt32();

                    std::uint16_t dataLength = serializer.readUInt16();
                    bytesRead += 7; // 1 + 4 + 2 bytes for type, entityId, dataLength

                    if (dataLength > 0)
                    {
                        event.data.resize(dataLength);
                        serializer.readBytes(event.data.data(), dataLength);
                        bytesRead += dataLength;
                    }

                    events.push_back(std::move(event));
                }

                return events;
            }

        public:
            ///
            /// @brief Constructor
            ///
            HandlerPacket() = default;

            ///
            /// @brief Destructor
            ///
            ~HandlerPacket() = default;

            // Delete copy constructor and assignment operator
            HandlerPacket(const HandlerPacket &) = delete;
            HandlerPacket &operator=(const HandlerPacket &) = delete;

            ///
            /// @brief Set rate limiting interval
            /// @param interval Minimum time between packets per session
            ///
            void setRateLimitInterval(std::chrono::milliseconds interval) { rateLimitInterval_ = interval; }

            ///
            /// @brief Register CONNECT packet handler
            /// @param handler Callback function for CONNECT packets
            ///
            void onConnect(ConnectHandler handler) { connectHandler_ = std::move(handler); }

            ///
            /// @brief Register CONNECT_ACCEPT packet handler
            /// @param handler Callback function for CONNECT_ACCEPT packets
            ///
            void onConnectAccept(ConnectAcceptHandler handler) { connectAcceptHandler_ = std::move(handler); }

            ///
            /// @brief Register DISCONNECT packet handler
            /// @param handler Callback function for DISCONNECT packets
            ///
            void onDisconnect(DisconnectHandler handler) { disconnectHandler_ = std::move(handler); }

            ///
            /// @brief Register WORLD_STATE packet handler
            /// @param handler Callback function for WORLD_STATE packets
            ///
            void onWorldState(WorldStateHandler handler) { worldStateHandler_ = std::move(handler); }

            ///
            /// @brief Register PING packet handler
            /// @param handler Callback function for PING packets
            ///
            void onPing(PingHandler handler) { pingHandler_ = std::move(handler); }

            ///
            /// @brief Register PONG packet handler
            /// @param handler Callback function for PONG packets
            ///
            void onPong(PongHandler handler) { pongHandler_ = std::move(handler); }

            ///
            /// @brief Register ERROR packet handler
            /// @param handler Callback function for ERROR packets
            ///
            void onError(ErrorHandler handler) { errorHandler_ = std::move(handler); }

            ///
            /// @brief Register ENTITY_EVENT packet handler
            /// @param handler Callback function for ENTITY_EVENT packets
            ///
            void onEntityEvent(EntityEventHandler handler) { entityEventHandler_ = std::move(handler); }

            ///
            /// @brief Process a received packet
            /// @param data Raw packet data
            /// @param context Packet context information
            /// @return Processing result
            ///
            HandlerResult processPacket(const std::vector<std::uint8_t> &data, const PacketContext &context)
            {
                if (data.size() < sizeof(PacketHeader))
                {
                    updateStats(static_cast<PacketType>(0), HandlerResult::INVALID_PACKET, data.size());
                    return HandlerResult::INVALID_PACKET;
                }

                // Check rate limiting
                if (isRateLimited(context.sessionId, context.receiveTime))
                {
                    updateStats(static_cast<PacketType>(0), HandlerResult::RATE_LIMITED, data.size());
                    return HandlerResult::RATE_LIMITED;
                }

                try
                {
                    Serializer serializer(data);
                    PacketHeader header = serializer.deserializeHeader();

                    // Validate header
                    if (header.length > MAX_PAYLOAD || header.sessionId != context.sessionId)
                    {
                        updateStats(static_cast<PacketType>(header.type), HandlerResult::INVALID_PACKET, data.size());
                        return HandlerResult::INVALID_PACKET;
                    }

                    PacketType packetType = static_cast<PacketType>(header.type);
                    HandlerResult result = HandlerResult::UNSUPPORTED_TYPE;

                    switch (packetType)
                    {
                        case PacketType::CONNECT:
                            if (connectHandler_)
                            {
                                auto packet = serializer.deserializeConnect();
                                result = connectHandler_(packet, context);
                            }
                            break;

                        case PacketType::CONNECT_ACCEPT:
                            if (connectAcceptHandler_)
                            {
                                auto packet = serializer.deserializeConnectAccept();
                                result = connectAcceptHandler_(packet, context);
                            }
                            break;

                        case PacketType::DISCONNECT:
                            if (disconnectHandler_)
                            {
                                auto packet = serializer.deserializeDisconnect();
                                result = disconnectHandler_(packet, context);
                            }
                            break;

                        case PacketType::WORLD_STATE:
                            if (worldStateHandler_)
                            {
                                auto packet = serializer.deserializeWorldState();
                                result = worldStateHandler_(packet, context);
                            }
                            break;

                        case PacketType::PING:
                            if (pingHandler_)
                            {
                                auto packet = serializer.deserializePingPong();
                                result = pingHandler_(packet, context);
                            }
                            break;

                        case PacketType::PONG:
                            if (pongHandler_)
                            {
                                auto packet = serializer.deserializePingPong();
                                result = pongHandler_(packet, context);
                            }
                            break;

                        case PacketType::PACKET_ERROR:
                            if (errorHandler_)
                            {
                                auto packet = serializer.deserializeError();
                                result = errorHandler_(packet, context);
                            }
                            break;

                        case PacketType::ENTITY_EVENT:
                            if (entityEventHandler_)
                            {
                                auto events = parseEntityEvents(serializer, header.length);
                                result = entityEventHandler_(events, context);
                            }
                            break;

                        default:
                            result = HandlerResult::UNSUPPORTED_TYPE;
                            break;
                    }

                    updateStats(packetType, result, data.size());
                    return result;
                }
                catch (const std::exception &)
                {
                    updateStats(static_cast<PacketType>(0), HandlerResult::PROCESSING_ERROR, data.size());
                    return HandlerResult::PROCESSING_ERROR;
                }
            }

            ///
            /// @brief Get handler statistics
            /// @return Current statistics
            ///
            const HandlerStats &getStats() const { return stats_; }

            ///
            /// @brief Reset statistics
            ///
            void resetStats() { stats_ = HandlerStats{}; }

            ///
            /// @brief Clear rate limiting data for a session
            /// @param sessionId Session to clear
            ///
            void clearSession(std::uint32_t sessionId) { lastPacketTime_.erase(sessionId); }

            ///
            /// @brief Clear all rate limiting data
            ///
            void clearAllSessions() { lastPacketTime_.clear(); }
    };

} // namespace rnp
