///
/// @file Common.hpp
/// @brief This file contains common definitions and constants
/// @namespace utl
///

#pragma once
#include <filesystem>

#ifdef _WIN32
#define NOMINMAX
#define PLUGINS_EXTENSION ".dll"
#elif __linux__
#define PLUGINS_EXTENSION ".so"
#elif __APPLE__
#define PLUGINS_EXTENSION ".dylib"
#endif

namespace utl
{
    namespace Config
    {
        namespace Audio
        {
            inline constexpr auto DEFAULT_AUDIO_VOLUME = 0.5F;  // unused
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
            // Constantes utiles pour le jeu
            inline constexpr auto WINDOW_WIDTH = DEFAULT_WINDOW_WIDTH;
            inline constexpr auto WINDOW_HEIGHT = DEFAULT_WINDOW_HEIGHT;
            inline constexpr auto WINDOW_CENTER_X = WINDOW_WIDTH / 2.0f;
            inline constexpr auto WINDOW_CENTER_Y = WINDOW_HEIGHT / 2.0f;
            inline constexpr auto WINDOW_LEFT = 0.0f;
            inline constexpr auto WINDOW_RIGHT = static_cast<float>(WINDOW_WIDTH);
            inline constexpr auto WINDOW_TOP = 0.0f;
            inline constexpr auto WINDOW_BOTTOM = static_cast<float>(WINDOW_HEIGHT);
        } // namespace Window
        namespace Game
        {
            inline constexpr auto DEFAULT_VIDEO_QUALITY = 1;  // 0=Low, 1=Medium, 2=High
            inline constexpr auto DEFAULT_CONTROL_SCHEME = 2; // 0=WASD, 1=ZQSD, 2=Arrows
            inline constexpr auto DEFAULT_SKIN_INDEX = 0;     // 0-4 different ship skins
        } // namespace Game
    } // namespace Config
    namespace Path
    {
        namespace Audio
        {
            inline constexpr auto AUDIO_TITLE = "assets/audio/title.mp3";
            inline constexpr auto AUDIO_COIN = "assets/audio/coin.mp3";
            inline constexpr auto AUDIO_BATTLE_THEME = "assets/audio/battle_theme.mp3";
            inline constexpr auto AUDIO_INPUT = "assets/audio/input.wav";
            inline constexpr auto AUDIO_DEATH_ALLIES = "assets/audio/deathallies.wav";
            inline constexpr auto AUDIO_DEATH_ENEMIES = "assets/audio/deathennemies.wav";
            inline constexpr auto AUDIO_SUPERCHARGED_SHOT = "assets/audio/shoot.wav";
            inline constexpr auto AUDIO_BEGIN = "assets/audio/begin.wav";
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
            inline constexpr auto TEXTURE_EXPLOSION = "assets/sprites/r-typesheet44.gif";
            inline constexpr auto TEXTURE_STAGE1_FLOOR = "assets/sprites/floorstage1.png";
            inline constexpr auto TEXTURE_STAGE1_CEILING = "assets/sprites/ceillingstage1.png";
            // HUD score
            inline constexpr auto TEXTURE_SCORE_COUNTER_BG = "assets/sprites/counter.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_0 = "assets/sprites/counter0.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_1 = "assets/sprites/counter1.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_2 = "assets/sprites/counter2.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_3 = "assets/sprites/counter3.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_4 = "assets/sprites/counter4.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_5 = "assets/sprites/counter5.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_6 = "assets/sprites/counter6.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_7 = "assets/sprites/counter7.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_8 = "assets/sprites/counter8.png";
            inline constexpr auto TEXTURE_SCORE_DIGIT_9 = "assets/sprites/counter9.png";
        } // namespace Texture
    } // namespace Path
} // namespace utl
