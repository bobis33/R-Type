///
/// @file Logger.hpp
/// @brief This file contains the Logger class
/// @namespace utl
///

#pragma once

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>

namespace utl
{

    enum class LogLevel : uint8_t
    {
        INFO,
        WARNING
    };

    ///
    /// @class Logger
    /// @brief Class for logging
    /// @namespace utl
    ///
    class Logger
    {

        public:
            Logger(const Logger &) = delete;
            Logger &operator=(const Logger &) = delete;
            Logger(Logger &&) = delete;
            Logger &operator=(Logger &&) = delete;

            static void init();

            template <typename Func> static void logExecutionTime(const std::string &message, Func &&func)
            {
                const auto start = std::chrono::high_resolution_clock::now();
                func();
                const auto end = std::chrono::high_resolution_clock::now();
                const auto duration = std::chrono::duration<float, std::milli>(end - start).count();

                std::cout << getColorForDuration(duration)
                          << formatLogMessage(LogLevel::INFO, message + " took " + std::to_string(duration) + " ms")
                          << LOG_LEVEL_COLOR[COLOR_RESET];
            }

            static void log(const std::string &message, const LogLevel &logLevel)
            {
                std::cout << (logLevel == LogLevel::INFO ? LOG_LEVEL_COLOR[COLOR_INFO] : LOG_LEVEL_COLOR[COLOR_WARNING])
                          << formatLogMessage(logLevel, message) << LOG_LEVEL_COLOR[COLOR_RESET];
            }

        private:
            enum ColorIndex : uint8_t
            {
                COLOR_ERROR,
                COLOR_INFO,
                COLOR_WARNING,
                COLOR_RESET
            };

            static constexpr std::array<const char *, 4> LOG_LEVEL_COLOR = {
                "\033[31m", // ERROR/slow execution
                "\033[32m", // INFO/fast execution
                "\033[33m", // WARNING/medium execution
                "\033[0m\n" // RESET + newline
            };

            static constexpr std::array<const char *, 2> LOG_LEVEL_STRING = {"INFO", "WARNING"};

            Logger() = default;
            ~Logger() = default;

            [[nodiscard]] static const char *getColorForDuration(const float duration)
            {
                return duration < 20.0F
                           ? LOG_LEVEL_COLOR[COLOR_INFO]
                           : (duration < 90.0F ? LOG_LEVEL_COLOR[COLOR_WARNING] : LOG_LEVEL_COLOR[COLOR_ERROR]);
            }

            [[nodiscard]] static std::string formatLogMessage(LogLevel level, const std::string &message)
            {
                const auto inTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::ostringstream ss;
                ss << "[" << std::put_time(std::localtime(&inTime), "%Y-%m-%d %X") << "] ";
                ss << "[" << LOG_LEVEL_STRING[static_cast<uint8_t>(level)] << "] " << message;
                return ss.str();
            }

    }; // class Logger

} // namespace utl
