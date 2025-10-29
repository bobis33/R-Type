///
/// @file AppConfigClient.hpp
/// @brief This file contains the app config structure for client
/// @namespace utl
///

#pragma once

#include "Utils/Common.hpp"

namespace utl
{
    namespace cli
    {
        struct AppConfig
        {
                unsigned int width = Config::Window::DEFAULT_WINDOW_WIDTH;
                unsigned int height = Config::Window::DEFAULT_WINDOW_HEIGHT;
                unsigned int frameLimit = Config::Window::DEFAULT_WINDOW_FRAME_LIMIT;
                bool fullscreen = Config::Window::DEFAULT_WINDOW_FULLSCREEN;
                std::string host = Config::Network::DEFAULT_NETWORK_HOST;
                unsigned int port = Config::Network::DEFAULT_NETWORK_PORT;
                std::string player_name = Config::Player::DEFAULT_PLAYER_NAME;
                float audioVolume = Config::Audio::DEFAULT_AUDIO_VOLUME;
                int videoQuality = Config::Game::DEFAULT_VIDEO_QUALITY;
                int controlScheme = Config::Game::DEFAULT_CONTROL_SCHEME;
                int skinIndex = Config::Game::DEFAULT_SKIN_INDEX;
        };
    } // namespace cli
} // namespace utl
