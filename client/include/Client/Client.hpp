///
/// @file Client.hpp
/// @brief This file contains the Client class declaration
/// @namespace cli
///

#pragma once

#include <unordered_map>

#include "Client/ArgsHandler.hpp"
#include "Engine/Engine.hpp"
#include "Interfaces/IGameClient.hpp"
#include "Utils/PluginLoader.hpp"

namespace cli
{

    struct AppConfig
    {
        unsigned int width = Config::Window::DEFAULT_WINDOW_WIDTH;
        unsigned int height = Config::Window::DEFAULT_WINDOW_HEIGHT;
        unsigned int frameLimit = Config::Window::DEFAULT_WINDOW_FRAME_LIMIT;
        bool fullscreen = Config::Window::DEFAULT_WINDOW_FULLSCREEN;
        std::string host = Config::Network::DEFAULT_NETWORK_HOST;
        uint16_t port = Config::Network::DEFAULT_NETWORK_PORT;
    }; // struct Config

    ///
    /// @class Client
    /// @brief Class for the client
    /// @namespace cli
    ///
    class Client
    {

        public:
            explicit Client(const ArgsConfig &cfg);
            ~Client() = default;

            Client(const Client &) = delete;
            Client &operator=(const Client &) = delete;
            Client(Client &&) = delete;
            Client &operator=(Client &&) = delete;

            void run();
            void stop() const;

        private:
            void handleEvents(eng::Event &event);
            AppConfig setupConfig(const ArgsConfig &cfg);
            void setupScenes() const;

            std::unique_ptr<utl::PluginLoader> m_pluginLoader;
            std::unique_ptr<eng::Engine> m_engine;
            std::shared_ptr<gme::IGameClient> m_gameSolo;
            std::shared_ptr<gme::IGameClient> m_gameMulti;
            std::unordered_map<eng::Key, bool> m_keysPressed;

            AppConfig m_config;
    }; // class Client

} // namespace cli