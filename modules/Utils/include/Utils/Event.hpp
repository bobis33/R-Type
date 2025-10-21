///
/// @file Event.hpp
/// @brief Event structures and types for event-driven communication
/// @namespace utl
///

#pragma once

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace utl
{

    ///
    /// @brief Event types for inter-component communication
    ///
    enum class EventType : std::uint32_t
    {
        // Network → Game events (0x1000 - 0x1FFF)
        WORLD_STATE_RECEIVED = 0x1000,
        ENTITY_EVENTS_RECEIVED = 0x1001,
        PLAYER_CONNECTED = 0x1002,
        PLAYER_DISCONNECTED = 0x1003,
        PLAYER_INPUT_RECEIVED = 0x1010,
        ENTITY_EVENT_RECEIVED = 0x1011,
        CONNECTION_ACCEPTED = 0x1020,
        CONNECTION_REJECTED = 0x1021,
        SERVER_DISCONNECTED = 0x1022,

        // Game → Network events (0x2000 - 0x2FFF)
        SEND_PLAYER_INPUT = 0x2000,
        SEND_ENTITY_EVENT = 0x2001,
        BROADCAST_WORLD_STATE = 0x2002,
        SEND_TO_CLIENT = 0x2010,
        REQUEST_CONNECT = 0x2020,
        REQUEST_DISCONNECT = 0x2021,

        // Audio events (0x3000 - 0x3FFF)
        PLAY_SOUND = 0x3000,
        STOP_SOUND = 0x3001,
        PAUSE_SOUND = 0x3002,
        RESUME_SOUND = 0x3003,
        SET_VOLUME = 0x3010,
        PLAY_MUSIC = 0x3020,
        STOP_MUSIC = 0x3021,

        // Renderer events (0x4000 - 0x4FFF)
        SPAWN_VISUAL_EFFECT = 0x4000,
        UPDATE_CAMERA = 0x4001,
        RENDER_TEXT = 0x4002,
        RENDER_SPRITE = 0x4003,
        UPDATE_ANIMATION = 0x4010,
        SCREEN_RESIZE = 0x4020,

        // System events (0x5000 - 0x5FFF)
        PAUSE_GAME = 0x5000,
        RESUME_GAME = 0x5001,
        QUIT_GAME = 0x5002,
        RESTART_GAME = 0x5003,
        SAVE_GAME = 0x5010,
        LOAD_GAME = 0x5011,

        // Input events (0x6000 - 0x6FFF)
        KEY_PRESSED = 0x6000,
        KEY_RELEASED = 0x6001,
        MOUSE_MOVED = 0x6002,
        MOUSE_CLICKED = 0x6003,
        MOUSE_RELEASED = 0x6004,
        CONTROLLER_INPUT = 0x6010,

        // Scene events (0x7000 - 0x7FFF)
        SCENE_CHANGE = 0x7000,
        SCENE_LOADED = 0x7001,
        SCENE_UNLOADED = 0x7002,
        MENU_OPTION_SELECTED = 0x7010,

        // Custom/Plugin events (0x8000 - 0xFFFF)
        CUSTOM_EVENT = 0x8000
    };

    ///
    /// @brief Event priority levels
    ///
    enum class EventPriority : std::uint8_t
    {
        LOW = 0,
        NORMAL = 1,
        HIGH = 2,
        CRITICAL = 3
    };

    ///
    /// @brief Event structure for inter-component communication
    ///
    struct Event
    {
            EventType type;                                  ///< Type of the event
            std::uint32_t sourceId = 0;                      ///< ID of the component that sent the event (0 = system)
            std::uint32_t targetId = 0;                      ///< ID of the target component (0 = broadcast)
            EventPriority priority = EventPriority::NORMAL;  ///< Event priority
            std::chrono::steady_clock::time_point timestamp; ///< Timestamp when event was created
            std::vector<std::uint8_t> data;                  ///< Serialized event data

            ///
            /// @brief Default constructor with current timestamp
            ///
            Event() : timestamp(std::chrono::steady_clock::now()) {}

            ///
            /// @brief Constructor with event type
            /// @param t Event type
            ///
            explicit Event(EventType t) : type(t), timestamp(std::chrono::steady_clock::now()) {}

            ///
            /// @brief Constructor with event type and data
            /// @param t Event type
            /// @param d Event data
            ///
            Event(EventType t, const std::vector<std::uint8_t> &d)
                : type(t), data(d), timestamp(std::chrono::steady_clock::now())
            {
            }

            ///
            /// @brief Constructor with all parameters
            /// @param t Event type
            /// @param src Source component ID
            /// @param tgt Target component ID (0 for broadcast)
            /// @param prio Event priority
            ///
            Event(EventType t, std::uint32_t src, std::uint32_t tgt = 0, EventPriority prio = EventPriority::NORMAL)
                : type(t), sourceId(src), targetId(tgt), priority(prio), timestamp(std::chrono::steady_clock::now())
            {
            }

            ///
            /// @brief Get event age in milliseconds
            /// @return Age in milliseconds
            ///
            std::uint64_t getAge() const
            {
                auto now = std::chrono::steady_clock::now();
                return std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count();
            }

            ///
            /// @brief Check if event has expired
            /// @param maxAgeMs Maximum age in milliseconds
            /// @return True if event has expired
            ///
            bool hasExpired(std::uint64_t maxAgeMs = 5000) const { return getAge() > maxAgeMs; }

            ///
            /// @brief Check if event is targeted to a specific component
            /// @return True if event has a specific target
            ///
            bool isTargeted() const { return targetId != 0; }

            ///
            /// @brief Check if event is a broadcast
            /// @return True if event is broadcast to all components
            ///
            bool isBroadcast() const { return targetId == 0; }
    };

    ///
    /// @brief Event statistics structure
    ///
    struct EventStats
    {
            std::uint64_t totalEventsPublished = 0;                         ///< Total events published
            std::uint64_t totalEventsConsumed = 0;                          ///< Total events consumed
            std::uint64_t totalEventsExpired = 0;                           ///< Total events that expired
            std::uint64_t currentQueueSize = 0;                             ///< Current queue size
            std::uint64_t maxQueueSize = 0;                                 ///< Maximum queue size reached
            std::chrono::steady_clock::time_point startTime;                ///< Statistics start time
            std::unordered_map<EventType, std::uint64_t> eventTypeCount;    ///< Count per event type
            std::unordered_map<EventPriority, std::uint64_t> priorityCount; ///< Count per priority
            std::unordered_map<std::uint32_t, std::uint64_t> sourceCount;   ///< Count per source component
            std::unordered_map<std::uint32_t, std::uint64_t> targetCount;   ///< Count per target component

            ///
            /// @brief Reset all statistics
            ///
            void reset()
            {
                totalEventsPublished = 0;
                totalEventsConsumed = 0;
                totalEventsExpired = 0;
                currentQueueSize = 0;
                maxQueueSize = 0;
                startTime = std::chrono::steady_clock::now();
                eventTypeCount.clear();
                priorityCount.clear();
                sourceCount.clear();
                targetCount.clear();
            }

            ///
            /// @brief Get events per second rate
            /// @return Events per second
            ///
            double getEventsPerSecond() const
            {
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime);
                if (elapsed.count() == 0)
                    return 0.0;
                return static_cast<double>(totalEventsPublished) / elapsed.count();
            }
    };

} // namespace utl
