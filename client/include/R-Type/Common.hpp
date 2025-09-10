///
/// @file Common.hpp
/// @brief This file contains common definitions and constants
/// @namespace rtp
///

#pragma once

namespace rtp
{
    namespace Paths
    {
        namespace Audio
        {
            inline constexpr auto AUDIO_TITLE = "assets/audio/title.mp3";
            inline constexpr auto AUDIO_COIN = "assets/audio/coin.mp3";
            inline constexpr auto AUDIO_BATTLE_THEME = "assets/audio/battle_theme.mp3";
        } // namespace Audio
        namespace Fonts
        {
            inline constexpr auto FONTS_RTYPE = "assets/fonts/r-type.otf";
        } // namespace Fonts
    } // namespace Paths
    namespace Config
    {
        namespace Window
        {
            inline constexpr auto DEFAULT_WINDOW_WIDTH = 960;
            inline constexpr auto DEFAULT_WINDOW_HEIGHT = 540;
            inline constexpr auto DEFAULT_WINDOW_FRAME_LIMIT = 240;
            inline constexpr auto DEFAULT_WINDOW_FULLSCREEN = false;
        } // namespace Window
        namespace Audio
        {
            inline constexpr auto DEFAULT_AUDIO_VOLUME = 50;   // unused
            inline constexpr auto DEFAULT_AUDIO_MUTED = false; // unused
        } // namespace Audio
    } // namespace Config
} // namespace rtp
