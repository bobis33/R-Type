///
/// @file AppConfigServer.hpp
/// @brief This file contains the app config structure for server
/// @namespace utl
///

#pragma once

#include "Utils/Common.hpp"

namespace utl
{
    namespace srv
    {
        struct AppConfig
        {
            std::string host;
            uint16_t port;
        };
    } // namespace srv
} // namespace utl
