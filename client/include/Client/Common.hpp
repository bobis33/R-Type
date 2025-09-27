///
/// @file Common.hpp
/// @brief This file contains common definitions and constants
/// @namespace cli
///

#pragma once

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
        namespace Texture
        {
            inline constexpr auto TEXTURE_PLAYER = "assets/sprites/r-typesheet42.gif";
        } // namespace Texture
    } // namespace Path
} // namespace cli
