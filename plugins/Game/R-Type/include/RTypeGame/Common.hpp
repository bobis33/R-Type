///
/// @file Common.hpp
/// @brief Common definitions and constants for R-Type game
/// @namespace gme
///

#pragma once

#include "Interfaces/IRenderer.hpp"

namespace gme
{
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
            inline constexpr auto TEXTURE_SHOOT = "assets/sprites/shoot.gif";
            inline constexpr auto TEXTURE_SHOOT_CHARGED = "assets/sprites/shootcharged.gif";
            inline constexpr auto TEXTURE_SHOOT_LOADING = "assets/sprites/shootchargedloading.gif";
            inline constexpr auto TEXTURE_ENEMY_EASY = "assets/sprites/r-typesheet5.gif";
            inline constexpr auto TEXTURE_ASTEROID = "assets/sprites/r-typesheet3.gif";
            inline constexpr auto TEXTURE_EXPLOSION = "assets/sprites/r-typesheet44.gif";
        } // namespace Texture
    } // namespace Path

    namespace Color
    {
        inline constexpr eng::Color WHITE = {.r = 255U, .g = 255U, .b = 255U, .a = 255U};
        inline constexpr eng::Color WHITE_TRANS = {.r = 255U, .g = 255U, .b = 255U, .a = 100U};
        inline constexpr eng::Color BLUE = {.r = 200U, .g = 200U, .b = 255U, .a = 150U};
        inline constexpr eng::Color BLUE_SECOND = {.r = 50U, .g = 100U, .b = 200U, .a = 60U};
        inline constexpr eng::Color YELLOW = {.r = 255U, .g = 255U, .b = 200U, .a = 200U};
        inline constexpr eng::Color PURPLE = {.r = 100U, .g = 50U, .b = 150U, .a = 80U};
        inline constexpr eng::Color GREEN = {.r = 200U, .g = 255U, .b = 200U, .a = 180U};
    } // namespace Color

} // namespace gme

