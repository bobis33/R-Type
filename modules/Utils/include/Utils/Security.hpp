///
/// @file Security.hpp
/// @brief Security utilities and input validation
/// @namespace utl
///

#pragma once

#include <algorithm>
#include <cctype>
#include <string>

namespace utl
{
    ///
    /// @class InputValidator
    /// @brief Input validation utilities for security-critical data
    /// @namespace utl
    ///
    class InputValidator final
    {
        public:
            InputValidator() = delete;
            ~InputValidator() = delete;
            InputValidator(const InputValidator &) = delete;
            InputValidator &operator=(const InputValidator &) = delete;
            InputValidator(InputValidator &&) = delete;
            InputValidator &operator=(InputValidator &&) = delete;

            ///
            /// @brief Validate player name for security and format compliance
            /// @param name Player name to validate
            /// @return True if valid (alphanumeric + spaces, max 32 chars), false otherwise
            /// @security Prevents injection attacks and buffer overflow
            ///
            [[nodiscard]] static bool isValidPlayerName(const std::string &name)
            {
                // Maximum length check
                if (name.empty() || name.length() > 32)
                {
                    return false;
                }

                // Only alphanumeric and spaces allowed
                return std::all_of(name.begin(), name.end(),
                                   [](unsigned char c) { return std::isalnum(c) || c == ' '; });
            }

            ///
            /// @brief Validate lobby name for security and format compliance
            /// @param name Lobby name to validate
            /// @return True if valid (alphanumeric + spaces + dash, max 32 chars), false otherwise
            /// @security Prevents injection attacks and buffer overflow
            ///
            [[nodiscard]] static bool isValidLobbyName(const std::string &name)
            {
                // Maximum length check
                if (name.empty() || name.length() > 32)
                {
                    return false;
                }

                // Only alphanumeric, spaces, and dashes allowed
                return std::all_of(name.begin(), name.end(),
                                   [](unsigned char c) { return std::isalnum(c) || c == ' ' || c == '-'; });
            }

            ///
            /// @brief Validate server IP address format (basic check)
            /// @param ip IP address string to validate
            /// @return True if plausible IPv4 format, false otherwise
            /// @security Prevents invalid IP configurations
            ///
            [[nodiscard]] static bool isValidIPAddress(const std::string &ip)
            {
                if (ip.empty() || ip.length() > 15)
                {
                    return false;
                }

                // Basic check: only digits and dots
                return std::all_of(ip.begin(), ip.end(), [](char c) { return std::isdigit(c) || c == '.'; });
            }

            ///
            /// @brief Validate port number range
            /// @param port Port number to validate
            /// @return True if port in valid range (1-65535), false otherwise
            /// @security Prevents invalid port configuration
            ///
            [[nodiscard]] static bool isValidPort(std::uint16_t port) { return port > 0 && port <= 65535; }

            ///
            /// @brief Sanitize string by removing dangerous characters
            /// @param input Input string to sanitize
            /// @param maxLength Maximum output length
            /// @return Sanitized string containing only safe characters
            /// @security Removes potential injection vectors
            ///
            [[nodiscard]] static std::string sanitize(const std::string &input, size_t maxLength = 32)
            {
                std::string result;
                result.reserve(std::min(input.length(), maxLength));

                for (unsigned char c : input)
                {
                    if (std::isalnum(c) || c == ' ' || c == '-' || c == '_')
                    {
                        if (result.length() < maxLength)
                        {
                            result.push_back(c);
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                return result;
            }
    };

    ///
    /// @namespace SecurityConfig
    /// @brief Security-related configuration constants
    ///
    namespace SecurityConfig
    {
        /// @brief Maximum player name length
        inline constexpr size_t MAX_PLAYER_NAME_LENGTH = 32;

        /// @brief Maximum lobby name length
        inline constexpr size_t MAX_LOBBY_NAME_LENGTH = 32;

        /// @brief Maximum packet size (bytes)
        inline constexpr size_t MAX_PACKET_SIZE = 512;

        /// @brief Maximum payload size (bytes)
        inline constexpr size_t MAX_PAYLOAD_SIZE = 512;

        /// @brief Client timeout duration (seconds)
        inline constexpr int CLIENT_TIMEOUT_SECONDS = 30;

        /// @brief Session timeout duration (minutes)
        inline constexpr int SESSION_TIMEOUT_MINUTES = 30;

        /// @brief Maximum packets per second per client
        inline constexpr size_t MAX_PACKETS_PER_SECOND = 100;

        /// @brief Maximum concurrent connections per IP
        inline constexpr size_t MAX_CLIENTS_PER_IP = 10;

        /// @brief HMAC size for message integrity (bytes) - SHA256
        inline constexpr size_t HMAC_SIZE = 32;

        /// @brief Minimum session ID value
        inline constexpr std::uint32_t MIN_SESSION_ID = 1;

        /// @brief Maximum session ID value
        inline constexpr std::uint32_t MAX_SESSION_ID = 0xFFFFFFFF;

        /// @brief Maximum number of retry attempts
        inline constexpr int MAX_RETRY_ATTEMPTS = 3;

        /// @brief Connection attempt timeout (milliseconds)
        inline constexpr int CONNECTION_TIMEOUT_MS = 5000;
    } // namespace SecurityConfig

} // namespace utl
