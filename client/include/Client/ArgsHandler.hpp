///
/// @file ArgsHandler.hpp
/// @brief This file contains the ArgsHandler class declaration
/// @namespace cli
///

#pragma once

#include <nlohmann/json.hpp>

#include "Utils/Common.hpp"

namespace cli
{

    using json = nlohmann::json;

    struct ArgsConfig
    {
            bool exit = false;
            unsigned int width = utl::Config::Window::DEFAULT_WINDOW_WIDTH;
            unsigned int height = utl::Config::Window::DEFAULT_WINDOW_HEIGHT;
            unsigned int frameLimit = utl::Config::Window::DEFAULT_WINDOW_FRAME_LIMIT;
            bool fullscreen = utl::Config::Window::DEFAULT_WINDOW_FULLSCREEN;
            std::string host = utl::Config::Network::DEFAULT_NETWORK_HOST;
            unsigned int port = utl::Config::Network::DEFAULT_NETWORK_PORT;
            std::string audio_lib_path = utl::Path::Plugin::PLUGIN_AUDIO_SFML.string();
            std::string network_lib_path = utl::Path::Plugin::PLUGIN_NETWORK_ASIO_CLIENT.string();
            std::string renderer_lib_path = utl::Path::Plugin::PLUGIN_RENDERER_SFML.string();
            std::string game_solo_lib_path = utl::Path::Plugin::PLUGIN_GAME_SOLO.string();
            std::string game_multi_lib_path = utl::Path::Plugin::PLUGIN_GAME_MULTI.string();

            static ArgsConfig fromFile(const std::string &path);
    }; // struct Config
    struct EnvConfig
    {
    };

    ///
    /// @class ArgsHandler
    /// @brief Class to handle command line arguments
    /// @namespace cli
    ///
    class ArgsHandler
    {

        public:
            ArgsHandler() = default;
            ~ArgsHandler() = default;

            ArgsHandler(const ArgsHandler &) = delete;
            ArgsHandler &operator=(const ArgsHandler &) = delete;
            ArgsHandler(ArgsHandler &&) = delete;
            ArgsHandler &operator=(ArgsHandler &&) = delete;

            static ArgsConfig ParseArgs(int argc, const char *const argv[]);
            static EnvConfig ParseEnv(const char *const env[]);

        private:
    }; // class ArgsHandler

} // namespace cli