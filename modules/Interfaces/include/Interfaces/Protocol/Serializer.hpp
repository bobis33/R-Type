///
/// @file Serializer.hpp
/// @brief Network packet serializer for RNP protocol
/// @namespace rnp
///

#pragma once

#include "Protocol.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace rnp
{

    ///
    /// @brief Utility class for endianness conversion
    ///
    class EndianUtils
    {
        public:
            static std::uint16_t hostToNetwork16(std::uint16_t hostValue)
            {
                return ((hostValue & 0xFF00) >> 8) | ((hostValue & 0x00FF) << 8);
            }

            static std::uint32_t hostToNetwork32(std::uint32_t hostValue)
            {
                return ((hostValue & 0xFF000000) >> 24) | ((hostValue & 0x00FF0000) >> 8) |
                       ((hostValue & 0x0000FF00) << 8) | ((hostValue & 0x000000FF) << 24);
            }

            static float hostToNetworkFloat(float hostValue)
            {
                std::uint32_t intValue;
                std::memcpy(&intValue, &hostValue, sizeof(float));
                intValue = hostToNetwork32(intValue);
                float result;
                std::memcpy(&result, &intValue, sizeof(float));
                return result;
            }

            static std::uint16_t networkToHost16(std::uint16_t networkValue)
            {
                return hostToNetwork16(networkValue); // Same operation for 16-bit
            }

            static std::uint32_t networkToHost32(std::uint32_t networkValue)
            {
                return hostToNetwork32(networkValue); // Same operation for 32-bit
            }

            static float networkToHostFloat(float networkValue)
            {
                return hostToNetworkFloat(networkValue); // Same operation for float
            }
    };

    ///
    /// @brief Binary serializer for RNP protocol packets
    ///
    class Serializer
    {
        private:
            std::vector<std::uint8_t> buffer_;
            std::size_t writePos_;
            std::size_t readPos_;

        public:
            ///
            /// @brief Constructor
            ///
            Serializer() : writePos_(0), readPos_(0) { buffer_.reserve(MAX_PAYLOAD + sizeof(PacketHeader)); }

            ///
            /// @brief Constructor with initial capacity
            /// @param capacity Initial buffer capacity
            ///
            explicit Serializer(std::size_t capacity) : writePos_(0), readPos_(0) { buffer_.reserve(capacity); }

            ///
            /// @brief Constructor from existing data
            /// @param data Existing data to deserialize
            ///
            explicit Serializer(const std::vector<std::uint8_t> &data)
                : buffer_(data), writePos_(data.size()), readPos_(0)
            {
            }

            ///
            /// @brief Reset the serializer for reuse
            ///
            void reset()
            {
                buffer_.clear();
                writePos_ = 0;
                readPos_ = 0;
            }

            ///
            /// @brief Get the serialized data
            /// @return Vector containing the serialized bytes
            ///
            const std::vector<std::uint8_t> &getData() const { return buffer_; }

            ///
            /// @brief Get the current size of serialized data
            /// @return Size in bytes
            ///
            std::size_t getSize() const { return writePos_; }

            ///
            /// @brief Write raw bytes
            /// @param data Pointer to data
            /// @param size Number of bytes to write
            ///
            void writeBytes(const void *data, std::size_t size)
            {
                if (writePos_ + size > MAX_PAYLOAD + sizeof(PacketHeader))
                {
                    throw std::runtime_error("Serializer buffer overflow");
                }

                buffer_.resize(writePos_ + size);
                std::memcpy(buffer_.data() + writePos_, data, size);
                writePos_ += size;
            }

            ///
            /// @brief Write a single byte
            /// @param value Byte value to write
            ///
            void writeByte(std::uint8_t value) { writeBytes(&value, sizeof(value)); }

            ///
            /// @brief Write a 16-bit integer (network byte order)
            /// @param value 16-bit integer to write
            ///
            void writeUInt16(std::uint16_t value)
            {
                std::uint16_t networkValue = EndianUtils::hostToNetwork16(value);
                writeBytes(&networkValue, sizeof(networkValue));
            }

            ///
            /// @brief Write a 32-bit integer (network byte order)
            /// @param value 32-bit integer to write
            ///
            void writeUInt32(std::uint32_t value)
            {
                std::uint32_t networkValue = EndianUtils::hostToNetwork32(value);
                writeBytes(&networkValue, sizeof(networkValue));
            }

            ///
            /// @brief Write a float (network byte order)
            /// @param value Float to write
            ///
            void writeFloat(float value)
            {
                float networkValue = EndianUtils::hostToNetworkFloat(value);
                writeBytes(&networkValue, sizeof(networkValue));
            }

            ///
            /// @brief Write a string with length prefix
            /// @param str String to write
            /// @param maxLength Maximum allowed length
            ///
            void writeString(const std::string &str, std::size_t maxLength)
            {
                if (str.length() > maxLength)
                {
                    throw std::runtime_error("String too long for serialization");
                }

                writeByte(static_cast<std::uint8_t>(str.length()));
                if (!str.empty())
                {
                    writeBytes(str.data(), str.length());
                }
            }

            ///
            /// @brief Read raw bytes
            /// @param data Pointer to destination buffer
            /// @param size Number of bytes to read
            ///
            void readBytes(void *data, std::size_t size)
            {
                if (readPos_ + size > buffer_.size())
                {
                    throw std::runtime_error("Serializer buffer underflow");
                }

                std::memcpy(data, buffer_.data() + readPos_, size);
                readPos_ += size;
            }

            ///
            /// @brief Read a single byte
            /// @return Byte value
            ///
            std::uint8_t readByte()
            {
                std::uint8_t value;
                readBytes(&value, sizeof(value));
                return value;
            }

            ///
            /// @brief Read a 16-bit integer (network byte order)
            /// @return 16-bit integer in host byte order
            ///
            std::uint16_t readUInt16()
            {
                std::uint16_t networkValue;
                readBytes(&networkValue, sizeof(networkValue));
                return EndianUtils::networkToHost16(networkValue);
            }

            ///
            /// @brief Read a 32-bit integer (network byte order)
            /// @return 32-bit integer in host byte order
            ///
            std::uint32_t readUInt32()
            {
                std::uint32_t networkValue;
                readBytes(&networkValue, sizeof(networkValue));
                return EndianUtils::networkToHost32(networkValue);
            }

            ///
            /// @brief Read a float (network byte order)
            /// @return Float in host byte order
            ///
            float readFloat()
            {
                float networkValue;
                readBytes(&networkValue, sizeof(networkValue));
                return EndianUtils::networkToHostFloat(networkValue);
            }

            ///
            /// @brief Read a string with length prefix
            /// @param maxLength Maximum expected length
            /// @return The read string
            ///
            std::string readString(std::size_t maxLength)
            {
                std::uint8_t length = readByte();
                if (length > maxLength)
                {
                    throw std::runtime_error("String length exceeds maximum");
                }

                if (length == 0)
                {
                    return std::string();
                }

                std::string result(length, '\0');
                readBytes(result.data(), length);
                return result;
            }

            ///
            /// @brief Serialize packet header
            /// @param header Header to serialize
            ///
            void serializeHeader(const PacketHeader &header)
            {
                writeByte(header.type);
                writeUInt16(header.length);
                writeUInt16(header.flags);
                writeUInt32(header.sessionId);
            }

            ///
            /// @brief Deserialize packet header
            /// @return Deserialized header
            ///
            PacketHeader deserializeHeader()
            {
                PacketHeader header;
                header.type = readByte();
                header.length = readUInt16();
                header.flags = readUInt16();
                header.sessionId = readUInt32();
                return header;
            }

            ///
            /// @brief Serialize CONNECT packet
            /// @param packet CONNECT packet to serialize
            ///
            void serializeConnect(const PacketConnect &packet)
            {
                writeByte(packet.nameLen);
                writeBytes(packet.playerName.data(), packet.playerName.size());
                writeUInt32(packet.clientCaps);
            }

            ///
            /// @brief Deserialize CONNECT packet
            /// @return Deserialized CONNECT packet
            ///
            PacketConnect deserializeConnect()
            {
                PacketConnect packet;
                packet.nameLen = readByte();
                readBytes(packet.playerName.data(), packet.playerName.size());
                packet.clientCaps = readUInt32();
                return packet;
            }

            ///
            /// @brief Serialize CONNECT_ACCEPT packet
            /// @param packet CONNECT_ACCEPT packet to serialize
            ///
            void serializeConnectAccept(const PacketConnectAccept &packet)
            {
                writeUInt32(packet.sessionId);
                writeUInt16(packet.tickRateHz);
                writeUInt16(packet.mtuPayloadBytes);
                writeUInt32(packet.serverCaps);
            }

            ///
            /// @brief Deserialize CONNECT_ACCEPT packet
            /// @return Deserialized CONNECT_ACCEPT packet
            ///
            PacketConnectAccept deserializeConnectAccept()
            {
                PacketConnectAccept packet;
                packet.sessionId = readUInt32();
                packet.tickRateHz = readUInt16();
                packet.mtuPayloadBytes = readUInt16();
                packet.serverCaps = readUInt32();
                return packet;
            }

            ///
            /// @brief Serialize DISCONNECT packet
            /// @param packet DISCONNECT packet to serialize
            ///
            void serializeDisconnect(const PacketDisconnect &packet) { writeUInt16(packet.reasonCode); }

            ///
            /// @brief Deserialize DISCONNECT packet
            /// @return Deserialized DISCONNECT packet
            ///
            PacketDisconnect deserializeDisconnect()
            {
                PacketDisconnect packet;
                packet.reasonCode = readUInt16();
                return packet;
            }

            ///
            /// @brief Serialize EntityState
            /// @param entity Entity state to serialize
            ///
            void serializeEntityState(const EntityState &entity)
            {
                writeUInt32(entity.id);
                writeUInt16(entity.type);
                writeFloat(entity.x);
                writeFloat(entity.y);
                writeFloat(entity.vx);
                writeFloat(entity.vy);
                writeByte(entity.stateFlags);
            }

            ///
            /// @brief Deserialize EntityState
            /// @return Deserialized entity state
            ///
            EntityState deserializeEntityState()
            {
                EntityState entity;
                entity.id = readUInt32();
                entity.type = readUInt16();
                entity.x = readFloat();
                entity.y = readFloat();
                entity.vx = readFloat();
                entity.vy = readFloat();
                entity.stateFlags = readByte();
                return entity;
            }

            ///
            /// @brief Serialize WORLD_STATE packet
            /// @param packet WORLD_STATE packet to serialize
            ///
            void serializeWorldState(const PacketWorldState &packet)
            {
                writeUInt32(packet.serverTick);
                writeUInt16(packet.entityCount);

                for (const auto &entity : packet.entities)
                {
                    serializeEntityState(entity);
                }
            }

            ///
            /// @brief Deserialize WORLD_STATE packet
            /// @return Deserialized WORLD_STATE packet
            ///
            PacketWorldState deserializeWorldState()
            {
                PacketWorldState packet;
                packet.serverTick = readUInt32();
                packet.entityCount = readUInt16();

                packet.entities.reserve(packet.entityCount);
                for (std::uint16_t i = 0; i < packet.entityCount; ++i)
                {
                    packet.entities.push_back(deserializeEntityState());
                }

                return packet;
            }

            ///
            /// @brief Serialize PING/PONG packet
            /// @param packet PING/PONG packet to serialize
            ///
            void serializePingPong(const PacketPingPong &packet)
            {
                writeUInt32(packet.nonce);
                writeUInt32(packet.sendTimeMs);
            }

            ///
            /// @brief Deserialize PING/PONG packet
            /// @return Deserialized PING/PONG packet
            ///
            PacketPingPong deserializePingPong()
            {
                PacketPingPong packet;
                packet.nonce = readUInt32();
                packet.sendTimeMs = readUInt32();
                return packet;
            }

            ///
            /// @brief Serialize ERROR packet
            /// @param packet ERROR packet to serialize
            ///
            void serializeError(const PacketError &packet)
            {
                writeUInt16(packet.errorCode);
                writeUInt16(packet.msgLen);
                if (!packet.description.empty())
                {
                    writeBytes(packet.description.data(), packet.description.length());
                }
            }

            ///
            /// @brief Deserialize ERROR packet
            /// @return Deserialized ERROR packet
            ///
            PacketError deserializeError()
            {
                PacketError packet;
                packet.errorCode = readUInt16();
                packet.msgLen = readUInt16();

                if (packet.msgLen > 0)
                {
                    packet.description.resize(packet.msgLen);
                    readBytes(packet.description.data(), packet.msgLen);
                }

                return packet;
            }

            ///
            /// @brief Serialize EventRecord for ENTITY_EVENT packets
            /// @param event Event record to serialize
            ///
            void serializeEventRecord(const EventRecord &event)
            {
                writeByte(static_cast<std::uint8_t>(event.type));
                writeUInt32(event.entityId);
                writeUInt16(static_cast<std::uint16_t>(event.data.size()));

                if (!event.data.empty())
                {
                    writeBytes(event.data.data(), event.data.size());
                }
            }

            ///
            /// @brief Deserialize EventRecord from ENTITY_EVENT packets
            /// @return Deserialized event record
            ///
            EventRecord deserializeEventRecord()
            {
                EventRecord event;
                event.type = static_cast<EventType>(readByte());
                event.entityId = readUInt32();

                std::uint16_t dataLength = readUInt16();
                if (dataLength > 0)
                {
                    event.data.resize(dataLength);
                    readBytes(event.data.data(), dataLength);
                }

                return event;
            }

            ///
            /// @brief Serialize multiple EventRecords for ENTITY_EVENT packet
            /// @param events Vector of event records to serialize
            ///
            void serializeEntityEvents(const std::vector<EventRecord> &events)
            {
                for (const auto &event : events)
                {
                    serializeEventRecord(event);
                }
            }

            ///
            /// @brief Deserialize multiple EventRecords from ENTITY_EVENT packet
            /// @param payloadSize Total size of the payload to deserialize
            /// @return Vector of deserialized event records
            ///
            std::vector<EventRecord> deserializeEntityEvents(std::size_t payloadSize)
            {
                std::vector<EventRecord> events;
                std::size_t bytesRead = 0;

                while (bytesRead < payloadSize && readPos_ < buffer_.size())
                {
                    std::size_t startPos = readPos_;
                    EventRecord event = deserializeEventRecord();
                    bytesRead += (readPos_ - startPos);
                    events.push_back(event);
                }

                return events;
            }

            ///
            /// @brief Serialize input data for INPUT events
            /// @param keys Keyboard input bitmask
            /// @param mouseX Mouse X position
            /// @param mouseY Mouse Y position
            /// @param mouseButtons Mouse button bitmask
            /// @param timestamp Input timestamp
            ///
            void serializeInputData(std::uint8_t keys, float mouseX, float mouseY, std::uint8_t mouseButtons,
                                    std::uint32_t timestamp)
            {
                writeByte(keys);
                writeFloat(mouseX);
                writeFloat(mouseY);
                writeByte(mouseButtons);
                writeUInt32(timestamp);
            }

            ///
            /// @brief Deserialize input data from INPUT events
            /// @param keys Output keyboard input bitmask
            /// @param mouseX Output mouse X position
            /// @param mouseY Output mouse Y position
            /// @param mouseButtons Output mouse button bitmask
            /// @param timestamp Output input timestamp
            ///
            void deserializeInputData(std::uint8_t &keys, float &mouseX, float &mouseY, std::uint8_t &mouseButtons,
                                      std::uint32_t &timestamp)
            {
                keys = readByte();
                mouseX = readFloat();
                mouseY = readFloat();
                mouseButtons = readByte();
                timestamp = readUInt32();
            }

            ///
            /// @brief Create an EventRecord for input data
            /// @param entityId Player entity ID
            /// @param keys Keyboard input
            /// @param mouseX Mouse X
            /// @param mouseY Mouse Y
            /// @param mouseButtons Mouse buttons
            /// @param timestamp Timestamp
            /// @return EventRecord ready to send
            ///
            static EventRecord createInputEvent(std::uint32_t entityId, std::uint8_t keys, float mouseX, float mouseY,
                                                std::uint8_t mouseButtons, std::uint32_t timestamp)
            {
                Serializer inputSerializer;
                inputSerializer.serializeInputData(keys, mouseX, mouseY, mouseButtons, timestamp);

                EventRecord event;
                event.type = EventType::INPUT;
                event.entityId = entityId;
                event.data = inputSerializer.getData();

                return event;
            }

            ///
            /// @brief Create an EventRecord for spawn event
            /// @param entityId Entity ID to spawn
            /// @param entityType Type of entity
            /// @param x X position
            /// @param y Y position
            /// @return EventRecord for spawn
            ///
            static EventRecord createSpawnEvent(std::uint32_t entityId, EntityType entityType, float x, float y)
            {
                Serializer spawnSerializer;
                spawnSerializer.writeUInt16(static_cast<std::uint16_t>(entityType));
                spawnSerializer.writeFloat(x);
                spawnSerializer.writeFloat(y);

                EventRecord event;
                event.type = EventType::SPAWN;
                event.entityId = entityId;
                event.data = spawnSerializer.getData();

                return event;
            }

            ///
            /// @brief Create an EventRecord for damage event
            /// @param entityId Entity taking damage
            /// @param damage Amount of damage
            /// @param sourceId Source entity causing damage
            /// @return EventRecord for damage
            ///
            static EventRecord createDamageEvent(std::uint32_t entityId, std::uint32_t damage,
                                                 std::uint32_t sourceId = 0)
            {
                Serializer damageSerializer;
                damageSerializer.writeUInt32(damage);
                damageSerializer.writeUInt32(sourceId);

                EventRecord event;
                event.type = EventType::DAMAGE;
                event.entityId = entityId;
                event.data = damageSerializer.getData();

                return event;
            }

            ///
            /// @brief Create an EventRecord for score event
            /// @param entityId Entity gaining score
            /// @param score Score amount
            /// @return EventRecord for score
            ///
            static EventRecord createScoreEvent(std::uint32_t entityId, std::uint32_t score)
            {
                Serializer scoreSerializer;
                scoreSerializer.writeUInt32(score);

                EventRecord event;
                event.type = EventType::SCORE;
                event.entityId = entityId;
                event.data = scoreSerializer.getData();

                return event;
            }

            ///
            /// @brief Create an EventRecord for despawn event
            /// @param entityId Entity to despawn
            /// @return EventRecord for despawn
            ///
            static EventRecord createDespawnEvent(std::uint32_t entityId)
            {
                EventRecord event;
                event.type = EventType::DESPAWN;
                event.entityId = entityId;
                // Despawn events don't need additional data

                return event;
            }
    };

} // namespace rnp
