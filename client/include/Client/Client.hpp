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
#include "Utils/AppConfigClient.hpp"
#include "Utils/PluginLoader.hpp"

namespace cli
{

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
            void updateKeyboardInput(ecs::Registry &registry);
            static utl::cli::AppConfig setupConfig(const ArgsConfig &cfg);
            void setupScenes();

            std::unique_ptr<utl::PluginLoader> m_pluginLoader;
            std::unique_ptr<eng::Engine> m_engine;
            std::shared_ptr<gme::IGameClient> m_gameSolo;
            std::shared_ptr<gme::IGameClient> m_gameMulti;
            std::unordered_map<eng::Key, bool> m_keysPressed;

            utl::cli::AppConfig m_config;
            bool m_showDebug = false;
    }; // class Client
} // namespace cli