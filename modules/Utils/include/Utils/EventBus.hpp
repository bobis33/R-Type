///
/// @file EventBus.hpp
/// @brief Thread-safe event bus implementation for inter-component communication
/// @namespace utl
///

#pragma once

#include "Interfaces/Protocol/Serializer.hpp"
#include "Utils/Event.hpp"
#include "Utils/Logger.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace utl
{

    ///
    /// @brief Thread-safe event bus for decoupled component communication
    ///
    class EventBus
    {
        private:
            ///
            /// @brief Internal event queue structure
            ///
            struct EventQueue
            {
                    std::queue<Event> events;
                    mutable std::mutex mutex;
                    std::condition_variable cv;
                    std::atomic<std::uint64_t> totalEvents{0};
            };

            ///
            /// @brief Priority queue for events
            ///
            struct PriorityEventQueue
            {
                    std::priority_queue<Event, std::vector<Event>, std::function<bool(const Event &, const Event &)>>
                        events;
                    mutable std::mutex mutex;

                    PriorityEventQueue()
                        : events([](const Event &a, const Event &b)
                                 { return static_cast<int>(a.priority) < static_cast<int>(b.priority); })
                    {
                    }
            };

            EventQueue m_eventQueue;
            PriorityEventQueue m_priorityQueue;
            std::unordered_map<EventType, std::unordered_set<std::uint32_t>> m_subscribers;
            mutable std::mutex m_subscribersMutex;
            EventStats m_stats;
            mutable std::mutex m_statsMutex;
            std::atomic<bool> m_running{true};
            std::atomic<std::uint64_t> m_maxQueueSize{10000};

            // Event filtering
            std::unordered_set<EventType> m_filteredTypes;
            mutable std::mutex m_filterMutex;

            // Component tracking
            std::unordered_map<std::uint32_t, std::string> m_componentNames;
            mutable std::mutex m_componentMutex;

        public:
            ///
            /// @brief Get singleton instance of EventBus
            /// @return Reference to the singleton EventBus instance
            ///
            static EventBus &getInstance()
            {
                static EventBus instance;
                return instance;
            }

            ///
            /// @brief Constructor
            ///
            EventBus()
            {
                m_stats.reset();
                utl::Logger::log("EventBus: Initialized", utl::LogLevel::INFO);
            }

            ///
            /// @brief Destructor
            ///
            ~EventBus()
            {
                m_running.store(false);
                m_eventQueue.cv.notify_all();
                utl::Logger::log("EventBus: Destroyed", utl::LogLevel::INFO);
            }

            // Delete copy constructor and assignment operator
            EventBus(const EventBus &) = delete;
            EventBus &operator=(const EventBus &) = delete;
            EventBus(EventBus &&) = delete;
            EventBus &operator=(EventBus &&) = delete;

            ///
            /// @brief Publish an event to the bus
            /// @param event Event to publish
            /// @return True if event was published successfully
            ///
            bool publish(const Event &event)
            {
                if (!m_running.load())
                {
                    return false;
                }

                // Check if event type is filtered
                {
                    std::lock_guard<std::mutex> lock(m_filterMutex);
                    if (m_filteredTypes.find(event.type) != m_filteredTypes.end())
                    {
                        return false; // Event type is filtered out
                    }
                }

                // Check queue size limit
                {
                    std::lock_guard<std::mutex> lock(m_eventQueue.mutex);
                    if (m_eventQueue.events.size() >= m_maxQueueSize.load())
                    {
                        utl::Logger::log("EventBus: Queue full, dropping event type " +
                                             std::to_string(static_cast<std::uint32_t>(event.type)),
                                         utl::LogLevel::WARNING);
                        return false;
                    }

                    m_eventQueue.events.push(event);
                    m_eventQueue.totalEvents++;
                }

                // Notify waiting consumers
                m_eventQueue.cv.notify_all();

                // Update statistics
                updateStats(event, true);

                utl::Logger::log("EventBus: Published event type " +
                                     std::to_string(static_cast<std::uint32_t>(event.type)) + " from source " +
                                     std::to_string(event.sourceId),
                                 utl::LogLevel::WARNING);

                return true;
            }

            ///
            /// @brief Publish event with serialized data
            /// @tparam T Type of data to serialize
            /// @param type Event type
            /// @param data Data to serialize
            /// @param sourceId Source component ID
            /// @param targetId Target component ID (0 for broadcast)
            /// @param priority Event priority
            /// @return True if event was published successfully
            ///
            template <typename T>
            bool publish(EventType type, const T &data, std::uint32_t sourceId = 0, std::uint32_t targetId = 0,
                         EventPriority priority = EventPriority::NORMAL)
            {
                Event event(type, sourceId, targetId, priority);

                // Serialize data using existing Serializer
                try
                {
                    rnp::Serializer serializer;
                    if constexpr (std::is_same_v<T, rnp::PacketWorldState>)
                    {
                        serializer.serializeWorldState(data);
                    }
                    else if constexpr (std::is_same_v<T, std::vector<rnp::EventRecord>>)
                    {
                        serializer.serializeEntityEvents(data);
                    }
                    else if constexpr (std::is_same_v<T, std::string>)
                    {
                        serializer.writeString(data);
                    }
                    else if constexpr (std::is_arithmetic_v<T>)
                    {
                        if constexpr (sizeof(T) == 1)
                        {
                            serializer.writeByte(static_cast<std::uint8_t>(data));
                        }
                        else if constexpr (sizeof(T) == 2)
                        {
                            serializer.writeUInt16(static_cast<std::uint16_t>(data));
                        }
                        else if constexpr (sizeof(T) == 4)
                        {
                            if constexpr (std::is_floating_point_v<T>)
                            {
                                serializer.writeFloat(static_cast<float>(data));
                            }
                            else
                            {
                                serializer.writeUInt32(static_cast<std::uint32_t>(data));
                            }
                        }
                    }
                    else
                    {
                        // For raw byte vectors, use directly
                        static_assert(std::is_same_v<T, std::vector<std::uint8_t>>,
                                      "Unsupported data type for EventBus serialization");
                        event.data = data;
                        return publish(event);
                    }

                    event.data = serializer.getData();
                    return publish(event);
                }
                catch (const std::exception &e)
                {
                    utl::Logger::log("EventBus: Failed to serialize data for event type " +
                                         std::to_string(static_cast<std::uint32_t>(type)) + " - " + e.what(),
                                     utl::LogLevel::WARNING);
                    return false;
                }
            }

            ///
            /// @brief Consume events from the bus (non-blocking)
            /// @param maxEvents Maximum number of events to consume
            /// @return Vector of consumed events
            ///
            std::vector<Event> consume(std::uint32_t maxEvents = 100)
            {
                std::vector<Event> events;

                std::lock_guard<std::mutex> lock(m_eventQueue.mutex);

                std::uint32_t count = 0;
                while (!m_eventQueue.events.empty() && count < maxEvents)
                {
                    Event event = m_eventQueue.events.front();
                    m_eventQueue.events.pop();

                    // Check if event has expired
                    if (event.hasExpired())
                    {
                        updateStats(event, false, true);
                        continue;
                    }

                    events.push_back(event);
                    count++;
                }

                // Update statistics
                for (const auto &event : events)
                {
                    updateStats(event, false);
                }

                return events;
            }

            ///
            /// @brief Consume events with timeout (blocking)
            /// @param timeout Maximum time to wait for events
            /// @param maxEvents Maximum number of events to consume
            /// @return Vector of consumed events
            ///
            std::vector<Event> waitForEvents(std::chrono::milliseconds timeout = std::chrono::milliseconds(100),
                                             std::uint32_t maxEvents = 100)
            {
                std::unique_lock<std::mutex> lock(m_eventQueue.mutex);

                // Wait for events or timeout
                if (m_eventQueue.cv.wait_for(lock, timeout, [this] { return !m_eventQueue.events.empty(); }))
                {
                    return consume(maxEvents);
                }

                return {}; // Timeout occurred
            }

            ///
            /// @brief Consume events of specific type
            /// @param type Event type to filter
            /// @param maxEvents Maximum number of events to consume
            /// @return Vector of events of the specified type
            ///
            std::vector<Event> consumeType(EventType type, std::uint32_t maxEvents = 100)
            {
                std::vector<Event> allEvents = consume(maxEvents);
                std::vector<Event> filteredEvents;

                for (const auto &event : allEvents)
                {
                    if (event.type == type)
                    {
                        filteredEvents.push_back(event);
                    }
                    else
                    {
                        // Put other events back in queue
                        std::lock_guard<std::mutex> lock(m_eventQueue.mutex);
                        m_eventQueue.events.push(event);
                    }
                }

                return filteredEvents;
            }

            ///
            /// @brief Consume events targeted to specific component
            /// @param targetId Target component ID
            /// @param maxEvents Maximum number of events to consume
            /// @return Vector of events targeted to the component
            ///
            std::vector<Event> consumeForTarget(std::uint32_t targetId, std::uint32_t maxEvents = 100)
            {
                std::vector<Event> allEvents = consume(maxEvents);
                std::vector<Event> filteredEvents;

                for (const auto &event : allEvents)
                {
                    if (event.targetId == targetId || event.isBroadcast())
                    {
                        filteredEvents.push_back(event);
                    }
                    else
                    {
                        // Put other events back in queue
                        std::lock_guard<std::mutex> lock(m_eventQueue.mutex);
                        m_eventQueue.events.push(event);
                    }
                }

                return filteredEvents;
            }

            ///
            /// @brief Subscribe component to specific event types
            /// @param componentId Component ID
            /// @param type Event type to subscribe to
            ///
            void subscribe(std::uint32_t componentId, EventType type)
            {
                std::lock_guard<std::mutex> lock(m_subscribersMutex);
                m_subscribers[type].insert(componentId);

                utl::Logger::log("EventBus: Component " + std::to_string(componentId) + " subscribed to event type " +
                                     std::to_string(static_cast<std::uint32_t>(type)),
                                 utl::LogLevel::WARNING);
            }

            ///
            /// @brief Unsubscribe component from specific event type
            /// @param componentId Component ID
            /// @param type Event type to unsubscribe from
            ///
            void unsubscribe(std::uint32_t componentId, EventType type)
            {
                std::lock_guard<std::mutex> lock(m_subscribersMutex);
                auto it = m_subscribers.find(type);
                if (it != m_subscribers.end())
                {
                    it->second.erase(componentId);
                    if (it->second.empty())
                    {
                        m_subscribers.erase(it);
                    }
                }

                utl::Logger::log("EventBus: Component " + std::to_string(componentId) +
                                     " unsubscribed from event type " +
                                     std::to_string(static_cast<std::uint32_t>(type)),
                                 utl::LogLevel::WARNING);
            }

            ///
            /// @brief Register component name for better debugging
            /// @param componentId Component ID
            /// @param name Component name
            ///
            void registerComponent(std::uint32_t componentId, const std::string &name)
            {
                std::lock_guard<std::mutex> lock(m_componentMutex);
                m_componentNames[componentId] = name;

                utl::Logger::log("EventBus: Registered component '" + name + "' with ID " + std::to_string(componentId),
                                 utl::LogLevel::INFO);
            }

            ///
            /// @brief Unregister component
            /// @param componentId Component ID
            ///
            void unregisterComponent(std::uint32_t componentId)
            {
                {
                    std::lock_guard<std::mutex> lock(m_componentMutex);
                    auto it = m_componentNames.find(componentId);
                    if (it != m_componentNames.end())
                    {
                        utl::Logger::log("EventBus: Unregistered component '" + it->second +
                                             "' (ID: " + std::to_string(componentId) + ")",
                                         utl::LogLevel::INFO);
                        m_componentNames.erase(it);
                    }
                }

                // Remove from all subscriptions
                std::lock_guard<std::mutex> lock(m_subscribersMutex);
                for (auto &pair : m_subscribers)
                {
                    pair.second.erase(componentId);
                }
            }

            ///
            /// @brief Add event type to filter (filtered events will be dropped)
            /// @param type Event type to filter
            ///
            void filterEventType(EventType type)
            {
                std::lock_guard<std::mutex> lock(m_filterMutex);
                m_filteredTypes.insert(type);

                utl::Logger::log("EventBus: Added filter for event type " +
                                     std::to_string(static_cast<std::uint32_t>(type)),
                                 utl::LogLevel::INFO);
            }

            ///
            /// @brief Remove event type from filter
            /// @param type Event type to unfilter
            ///
            void unfilterEventType(EventType type)
            {
                std::lock_guard<std::mutex> lock(m_filterMutex);
                m_filteredTypes.erase(type);

                utl::Logger::log("EventBus: Removed filter for event type " +
                                     std::to_string(static_cast<std::uint32_t>(type)),
                                 utl::LogLevel::INFO);
            }

            ///
            /// @brief Set maximum queue size
            /// @param maxSize Maximum number of events in queue
            ///
            void setMaxQueueSize(std::uint64_t maxSize) { m_maxQueueSize.store(maxSize); }

            ///
            /// @brief Get current queue size
            /// @return Current number of events in queue
            ///
            std::uint64_t getQueueSize() const
            {
                std::lock_guard<std::mutex> lock(m_eventQueue.mutex);
                return m_eventQueue.events.size();
            }

            ///
            /// @brief Get event bus statistics
            /// @return Copy of current statistics
            ///
            EventStats getStats() const
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);
                EventStats stats = m_stats;
                stats.currentQueueSize = getQueueSize();
                return stats;
            }

            ///
            /// @brief Clear all statistics
            ///
            void clearStats()
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);
                m_stats.reset();
            }

            ///
            /// @brief Clear all events from queue
            ///
            void clear()
            {
                std::lock_guard<std::mutex> lock(m_eventQueue.mutex);
                while (!m_eventQueue.events.empty())
                {
                    m_eventQueue.events.pop();
                }
                utl::Logger::log("EventBus: Cleared all events from queue", utl::LogLevel::INFO);
            }

            ///
            /// @brief Stop the event bus
            ///
            void stop()
            {
                m_running.store(false);
                m_eventQueue.cv.notify_all();
                utl::Logger::log("EventBus: Stopped", utl::LogLevel::INFO);
            }

            ///
            /// @brief Check if event bus is running
            /// @return True if running
            ///
            bool isRunning() const { return m_running.load(); }

        private:
            ///
            /// @brief Update statistics for an event
            /// @param event Event to update stats for
            /// @param isPublish True if this is a publish operation
            /// @param isExpired True if event expired
            ///
            void updateStats(const Event &event, bool isPublish, bool isExpired = false)
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);

                if (isPublish)
                {
                    m_stats.totalEventsPublished++;
                }
                else
                {
                    m_stats.totalEventsConsumed++;
                }

                if (isExpired)
                {
                    m_stats.totalEventsExpired++;
                }

                m_stats.eventTypeCount[event.type]++;
                m_stats.priorityCount[event.priority]++;
                m_stats.sourceCount[event.sourceId]++;

                if (event.targetId != 0)
                {
                    m_stats.targetCount[event.targetId]++;
                }

                // Update max queue size
                std::uint64_t currentSize = getQueueSize();
                if (currentSize > m_stats.maxQueueSize)
                {
                    m_stats.maxQueueSize = currentSize;
                }
            }
    };

} // namespace utl
