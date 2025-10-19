///
/// @file Common.hpp
/// @brief This file contains common definitions and constants
/// @namespace cli
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

namespace cli
{
    namespace Config
    {
        namespace Audio
        {
            inline constexpr auto DEFAULT_AUDIO_VOLUME = 50;   // unused
            inline constexpr auto DEFAULT_AUDIO_MUTED = false; // unused
        } // namespace Audio
        namespace Network
        {
            inline constexpr auto DEFAULT_NETWORK_HOST = "127.0.0.1";
            inline constexpr auto DEFAULT_NETWORK_PORT = 2560;
        } // namespace Network
        namespace Window
        {
            inline constexpr auto DEFAULT_WINDOW_WIDTH = 960;
            inline constexpr auto DEFAULT_WINDOW_HEIGHT = 540;
            inline constexpr auto DEFAULT_WINDOW_FRAME_LIMIT = 240;
            inline constexpr auto DEFAULT_WINDOW_FULLSCREEN = false;
        } // namespace Window
    } // namespace Config
    namespace Path
    {
        namespace Audio
        {
            inline constexpr auto AUDIO_TITLE = "assets/audio/title.mp3";
            inline constexpr auto AUDIO_COIN = "assets/audio/coin.mp3";
            inline constexpr auto AUDIO_BATTLE_THEME = "assets/audio/battle_theme.mp3";
        } // namespace Audio
        namespace Font
        {
            inline constexpr auto FONTS_RTYPE = "assets/fonts/r-type.otf";
        } // namespace Font
        namespace Plugin
        { // TODO(bobis33): dont use filesystem here
            inline auto PLUGIN_AUDIO_SFML =
                std::filesystem::path(PLUGINS_DIR) / ("audio_sfml" + std::string(PLUGINS_EXTENSION));
            inline auto PLUGIN_NETWORK_ASIO_CLIENT =
                std::filesystem::path(PLUGINS_DIR) / ("network_asio_client" + std::string(PLUGINS_EXTENSION));
            inline auto PLUGIN_RENDERER_SFML =
                std::filesystem::path(PLUGINS_DIR) / ("renderer_sfml" + std::string(PLUGINS_EXTENSION));
            inline auto PLUGIN_GAME_SOLO =
                std::filesystem::path(PLUGINS_DIR) / ("game_rtype_client_solo" + std::string(PLUGINS_EXTENSION));
            inline auto PLUGIN_GAME_MULTI =
                std::filesystem::path(PLUGINS_DIR) / ("game_rtype_client_multi" + std::string(PLUGINS_EXTENSION));
        } // namespace Plugin
        namespace Texture
        {
            inline constexpr auto TEXTURE_PLAYER = "assets/sprites/r-typesheet42.gif";
            inline constexpr auto TEXTURE_SHOOT = "assets/sprites/shoot.gif";
            inline constexpr auto TEXTURE_SHOOT_CHARGED = "assets/sprites/shootcharged.gif";
            inline constexpr auto TEXTURE_SHOOT_LOADING = "assets/sprites/shootchargedloading.gif";
            inline constexpr auto TEXTURE_ENEMY_EASY = "assets/sprites/r-typesheet5.gif";
            inline constexpr auto TEXTURE_ASTEROID = "assets/sprites/r-typesheet3.gif";
            inline constexpr auto TEXTURE_EXPLOSION = "assets/sprites/r-typesheet44.gif";
            inline constexpr auto TEXTURE_ARROW = "assets/icons/arrow.png";
            inline constexpr auto TEXTURE_MOON_BACK = "assets/parallax/moon_back.png";
            inline constexpr auto TEXTURE_MOON_EARTH = "assets/parallax/moon_earth.png";
            inline constexpr auto TEXTURE_MOON_FLOOR = "assets/parallax/moon_floor.png";
            inline constexpr auto TEXTURE_MOON_FRONT = "assets/parallax/moon_front.png";
            inline constexpr auto TEXTURE_MOON_MID = "assets/parallax/moon_mid.png";
        } // namespace Texture
    } // namespace Path
} // namespace cli
