///
/// @file INetworkClient.hpp
/// @brief This file contains the client network interface
/// @namespace eng
///

#pragma once

#include "Utils/Interfaces/IPlugin.hpp"

namespace eng
{

    ///
    /// @class INetworkClient
    /// @brief Interface for the client network
    /// @namespace eng
    ///
    class INetworkClient : public utl::IPlugin
    {
        public:
            virtual void connect(const std::string &host, unsigned short int port) = 0;
            virtual void disconnect() = 0;

        private:
    }; // class INetworkClient

} // namespace eng