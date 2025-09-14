///
/// @file Client.hpp
/// @brief This file contains the Client class declaration
/// @namespace cli
///

#pragma once

#include "Client/ArgsHandler.hpp"
#include "Engine/Engine.hpp"
#include "Interfaces/IGameClient.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Utils/Clock.hpp"

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

        private:
            std::unique_ptr<IGameClient> m_game;
            std::unique_ptr<eng::Engine> m_engine;
    }; // class Client

} // namespace cli