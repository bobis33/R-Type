///
/// @file Common.hpp
/// @brief This file contains common definitions and constants
/// @namespace srv
///

#pragma once

#include <filesystem>

#ifdef _WIN32
#define PLUGINS_EXTENSION ".dll"
#elif __linux__
#define PLUGINS_EXTENSION ".so"
#elif __APPLE__
#define PLUGINS_EXTENSION ".dylib"
#endif

namespace srv
{
    namespace Config::Network
    {
        inline constexpr auto DEFAULT_NETWORK_HOST = "0.0.0.0";
        inline constexpr auto DEFAULT_NETWORK_PORT = 2560;
        inline constexpr auto DEFAULT_MAX_CLIENT = 4;
    } // namespace Config::Network
    namespace Game
    {
        inline constexpr auto DEFAULT_TICK_RATE = 60;
        inline constexpr auto DEFAULT_UPDATE_INTERVAL = 1 / 20.F;
    } // namespace Game
    namespace Path::Plugin
    {
        inline auto PLUGINS_NETWORK_ASIO_SERVER =
            std::filesystem::path(PLUGINS_DIR) / ("network_asio_server" + std::string(PLUGINS_EXTENSION));
    }
} // namespace srv
