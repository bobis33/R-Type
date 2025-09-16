///
/// @file ArgsHandler.hpp
/// @brief This file contains the ArgsHandler class declaration
/// @namespace rtp
///

#pragma once

#include <fstream>

#include "nlohmann/json.hpp"

namespace rtp
{

    using json = nlohmann::json;

    struct ArgsConfig
    {
            bool exit = false;
            std::string host = "0.0.0.0";
            unsigned int port = 2560;

            static ArgsConfig fromFile(const std::string &path);
    }; // struct Config
    struct EnvConfig
    {
    };

    ///
    /// @class ArgsHandler
    /// @brief Class to handle command line arguments
    /// @namespace rtp
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

} // namespace rtp
