///
/// @file NetworkClient.hpp
/// @brief This file contains the client network implementation for Asio
/// @namespace eng
///

#pragma once

#include <string>

#include "Interfaces/INetworkClient.hpp"
#include "asio.hpp"

namespace eng
{

    ///
    /// @class NetworkClient
    /// @brief Network implementation with asio for client
    /// @namespace eng
    ///
    class NetworkClient final : public INetworkClient
    {
        public:
            NetworkClient(unsigned int port, const std::string &address);
            ~NetworkClient() override;

            NetworkClient(const NetworkClient &) = delete;
            NetworkClient(NetworkClient &&) = delete;
            NetworkClient &operator=(const NetworkClient &) = delete;
            NetworkClient &operator=(NetworkClient &&) = delete;

        private:
    }; // class NetworkClient
} // namespace eng
