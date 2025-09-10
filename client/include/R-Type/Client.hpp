///
/// @file Client.hpp
/// @brief This file contains the Client class declaration
/// @namespace rtp
///

#pragma once

#include "R-Type/ArgsHandler.hpp"
#include "R-Type/Renderer/Renderer.hpp"

namespace rtp
{

    ///
    /// @class Client
    /// @brief Class for the client
    /// @namespace rtp
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
            Renderer m_renderer;
    }; // class Client

} // namespace rtp