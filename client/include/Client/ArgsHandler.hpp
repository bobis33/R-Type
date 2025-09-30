///
/// @file ArgsHandler.hpp
/// @brief This file contains the ArgsHandler class declaration
/// @namespace cli
///

#pragma once

#include <nlohmann/json.hpp>

#include "Client/Common.hpp"

namespace cli
{

    using json = nlohmann::json;

    struct ArgsConfig
    {
            bool exit = false;
            unsigned int width = Config::Window::DEFAULT_WINDOW_WIDTH;
            unsigned int height = Config::Window::DEFAULT_WINDOW_HEIGHT;
            unsigned int frameLimit = Config::Window::DEFAULT_WINDOW_FRAME_LIMIT;
            bool fullscreen = Config::Window::DEFAULT_WINDOW_FULLSCREEN;
            std::string host = Config::Network::DEFAULT_NETWORK_HOST;
            unsigned int port = Config::Network::DEFAULT_NETWORK_PORT;
            std::string network_lib_path;
            std::string audio_lib_path;
            std::string renderer_lib_path;
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