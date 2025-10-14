///
/// @file ArgsHandler.hpp
/// @brief This file contains the ArgsHandler class declaration
/// @namespace srv
///

#pragma once

#include <fstream>

#include <nlohmann/json.hpp>

#include "Server/Common.hpp"

namespace srv
{

    using json = nlohmann::json;

    struct ArgsConfig
    {
            bool exit = false;
            std::string host = Config::Network::DEFAULT_NETWORK_HOST;
            uint16_t port = Config::Network::DEFAULT_NETWORK_PORT;
            std::string network_lib_path = Path::Plugin::PLUGINS_NETWORK_ASIO_SERVER.string();

            static ArgsConfig fromFile(const std::string &path);
    }; // struct Config
    struct EnvConfig
    {
    };

    ///
    /// @class ArgsHandler
    /// @brief Class to handle command line arguments
    /// @namespace srv
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

} // namespace srv