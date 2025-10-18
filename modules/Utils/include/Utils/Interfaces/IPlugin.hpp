///
/// @file IPlugin.hpp
/// @brief This file contains the plugin interface
/// @namespace utl
///

#pragma once

#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #define PLUGIN_EXPORT __declspec(dllexport)
#else
    #define PLUGIN_EXPORT
#endif

namespace utl
{

    enum class PluginType : uint8_t
    {
        AUDIO = 0,
        NETWORK_CLIENT = 1,
        NETWORK_SERVER = 1,
        RENDERER = 2,
        GAME_CLIENT = 3,
        GAME_SERVER = 4,
        UNKNOWN = 255
    };

    ///
    /// @interface IPlugin
    /// @brief Interface for plugins
    /// @namespace utl
    ///
    class IPlugin
    {

        public:
            virtual ~IPlugin() = default;
            [[nodiscard]] virtual const std::string getName() const = 0;
            [[nodiscard]] virtual PluginType getType() const = 0;

    }; // interface IPlugin
} // namespace utl
