///
/// @file EventHandler.hpp
/// @brief This file contains the EventHandler class declaration
/// @namespace rtp
///
#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>

#include <SFML/Window/Event.hpp>

namespace rtp
{
    ///
    /// @class EventHandler
    /// @brief Class for the EventHandler
    /// @namespace rtp
    ///
    class EventHandler
    {

        public:
            template <typename T> using Handler = std::function<void(const T &)>;

            EventHandler() = default;
            ~EventHandler() = default;

            EventHandler(const EventHandler &) = delete;
            EventHandler &operator=(const EventHandler &) = delete;
            EventHandler(EventHandler &&) = delete;
            EventHandler &operator=(EventHandler &&) = delete;

            template <typename T> void subscribe(Handler<T> handler)
            {
                auto &handlers = m_subscribers[std::type_index(typeid(T))];
                handlers.push_back(
                    [h = std::move(handler)](const sf::Event &e)
                    {
                        if (const auto *data = e.getIf<T>())
                        {
                            h(*data);
                        }
                    });
            }

            void publish(const sf::Event &e);

        private:
            using AnyHandler = std::function<void(const sf::Event &)>;
            std::unordered_map<std::type_index, std::vector<AnyHandler>> m_subscribers;

    }; // class EventHandler

} // namespace rtp