///
/// @file PluginLoader.hpp
/// @brief Modern, cross-platform plugin loader
/// @namespace utl
///

#pragma once

#include <concepts>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#define PLUGINS_EXTENSION ".dll"
#else
#include <dlfcn.h>
#if __APPLE__
#define PLUGINS_EXTENSION ".dylib"
#else
#define PLUGINS_EXTENSION ".so"
#endif
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

    /// Handle to a dynamic library with RAII
    struct SharedLib
    {
            LibHandle handle = nullptr;

            explicit SharedLib(const LibHandle h = nullptr) : handle(h) {}
            ~SharedLib() { close(); }

            SharedLib(const SharedLib &) = delete;
            SharedLib &operator=(const SharedLib &) = delete;
            SharedLib(SharedLib &&other) noexcept : handle(other.handle) { other.handle = nullptr; }
            SharedLib &operator=(SharedLib &&other) noexcept
            {
                if (this != &other)
                {
                    close();
                    handle = other.handle;
                    other.handle = nullptr;
                }
                return *this;
            }

            void close()
            {
                if (!handle)
                    return;
#ifdef _WIN32
                FreeLibrary(handle);
#else
                dlclose(handle);
#endif
                handle = nullptr;
            }

            explicit operator bool() const { return handle != nullptr; }
    };

    using EntryPointFn = IPlugin *(*)();

    ///
    /// @class PluginLoader
    /// @brief Modern, type-safe plugin loader
    /// @namespace utl
    ///
    class PluginLoader
    {
        public:
            PluginLoader() = default;
            ~PluginLoader() = default;

            PluginLoader(const PluginLoader &) = delete;
            PluginLoader &operator=(const PluginLoader &) = delete;
            PluginLoader(PluginLoader &&) = delete;
            PluginLoader &operator=(PluginLoader &&) = delete;

            ///
            /// Load a plugin of type T
            /// @tparam T Expected plugin interface (must derive from IPlugin)
            /// @param path Path to the dynamic library
            /// @return shared_ptr<T> instance
            ///
            template <std::derived_from<IPlugin> T> std::shared_ptr<T> loadPlugin(const std::string &path)
            {
                std::scoped_lock lock(m_mutex);

                if (m_plugins.contains(path))
                    throw std::runtime_error("Plugin already loaded: " + path);

                SharedLib lib = loadLibrary(path);
                const EntryPointFn entry = getEntryPoint(lib, path);

                std::unique_ptr<IPlugin> plugin(entry());
                if (!plugin)
                    throw std::runtime_error("EntryPoint failed: " + path);

                T *typed = dynamic_cast<T *>(plugin.get());
                if (!typed)
                    throw std::runtime_error("Plugin type mismatch: " + path);

                auto [it, inserted] = m_plugins.emplace(path, std::move(plugin));
                if (!inserted)
                    throw std::runtime_error("Failed to store plugin: " + path);

                m_handles[path] = std::move(lib);

                Logger::log("Plugin loaded:\t name: " + it->second->getName() + "\t path: " + path, LogLevel::INFO);

                std::shared_ptr<IPlugin> baseShared(it->second.get(), [](IPlugin *) {});
                return std::shared_ptr<T>(baseShared, typed);
            }

        private:
            std::mutex m_mutex;
            std::unordered_map<std::string, SharedLib> m_handles;
            std::unordered_map<std::string, std::unique_ptr<IPlugin>> m_plugins;

            SharedLib loadLibrary(const std::string &path)
            {

#ifdef _WIN32
                const LibHandle handle = LoadLibraryA(path.c_str());
                if (!handle)
                {
                    throw std::runtime_error("Cannot load library: " + path);
                }
#else
                dlerror();
                const LibHandle handle = dlopen(path.c_str(), RTLD_LAZY);
                if (!handle)
                {
                    const char *error = dlerror();
                    std::string msg = "Cannot load library: " + path;
                    if (error)
                    {
                        msg += " (" + std::string(error) + ")";
                    }
                    throw std::runtime_error(msg);
                }
#endif
                return SharedLib(handle);
            }

            EntryPointFn getEntryPoint(SharedLib &lib, const std::string &path)
            {
                EntryPointFn entry =
#ifdef _WIN32
                    reinterpret_cast<EntryPointFn>(GetProcAddress(lib.handle, "entryPoint"));
#else
                    reinterpret_cast<EntryPointFn>(dlsym(lib.handle, "entryPoint"));
#endif
                if (!entry)
                    throw std::runtime_error("EntryPoint not found in plugin: " + path);
                return entry;
            }
    };

} // namespace utl
