///
/// @file PluginLoader.hpp
/// @brief This file contains the PluginLoader class declaration
/// @namespace utl
///

#pragma once

#include <memory>
#include <ranges>
#include <stdexcept>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#define PLUGINS_EXTENSION ".dll"
#elif __linux__
#include <dlfcn.h>
#define PLUGINS_EXTENSION ".so"
#elif __APPLE__
#include <dlfcn.h>
#define PLUGINS_EXTENSION ".dylib"
#endif
#include "Utils/Interfaces/IPlugin.hpp"
#include "Utils/Logger.hpp"

namespace utl
{

    using LibHandle =
#ifdef _WIN32
        HMODULE;
#else
        void *;
#endif

    using EntryPointFn = std::unique_ptr<IPlugin> (*)();

    ///
    /// @class PluginLoader
    /// @brief Class for managing plugins
    /// @namespace utl
    ///
    class PluginLoader
    {
        public:
            PluginLoader() = default;

            ~PluginLoader()
            {
                m_plugins.clear();
                for (auto &handle : m_handles | std::views::values)
                {
#ifdef _WIN32
                    FreeLibrary(handle);
#else
                    dlclose(handle);
#endif
                }
            }

            PluginLoader(const PluginLoader &) = delete;
            PluginLoader &operator=(const PluginLoader &) = delete;
            PluginLoader(PluginLoader &&) = delete;
            PluginLoader &operator=(PluginLoader &&) = delete;

            template <typename T> std::shared_ptr<T> loadPlugin(const std::string &path)
            {
#ifdef _WIN32
                LibHandle handle = LoadLibraryA(path.c_str());
#else
                LibHandle handle = dlopen(path.c_str(), RTLD_LAZY);
#endif
                if (handle == nullptr)
                {
                    throw std::runtime_error("Impossible to load plugin: " + path);
                }

#ifdef _WIN32
                const auto entry = reinterpret_cast<EntryPointFn>(GetProcAddress(handle, "entryPoint"));
#else
                const auto entry = reinterpret_cast<EntryPointFn>(dlsym(handle, "entryPoint"));
#endif
                if (entry == nullptr)
                {
                    throw std::runtime_error("EntryPoint not found in plugin: " + path);
                }

                std::unique_ptr<IPlugin> plugin = entry();
                if (!plugin)
                {
                    throw std::runtime_error("EntryPoint failed to create plugin instance: " + path);
                }

                IPlugin *rawPtr = plugin.get();

                m_handles[path] = handle;
                m_plugins[path] = std::move(plugin);

                Logger::log("Plugin loaded:\t name: " + rawPtr->getName() + "\t path: " + path, LogLevel::INFO);
                T *typed = dynamic_cast<T *>(rawPtr);
                if (!typed)
                    throw std::runtime_error("Plugin type mismatch: " + path);
                return std::shared_ptr<T>(typed, [](T *) {});
            }

        private:
            std::unordered_map<std::string, LibHandle> m_handles;
            std::unordered_map<std::string, std::unique_ptr<IPlugin>> m_plugins;
    }; // class PluginLoader

} // namespace utl
