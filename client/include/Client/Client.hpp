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

        private:
            void handleEvents(eng::Event &event);

            std::unique_ptr<utl::PluginLoader> m_pluginLoader;
            std::unique_ptr<eng::Engine> m_engine;
            std::shared_ptr<gme::IGameClient> m_game;
            std::unordered_map<eng::Key, bool> m_keysPressed;
    }; // class Client

} // namespace cli