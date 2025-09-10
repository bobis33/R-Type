#include <fstream>
#include <functional>
#include <iostream>
#include <unordered_map>

#ifdef _WIN32
#define APP_EXTENSION ".exe"
#else
#define APP_EXTENSION ""
#endif

#include "R-Type/ArgsHandler.hpp"
#include "R-Type/Generated/Version.hpp"
#include "Utils/Logger.hpp"

static constexpr std::string_view HELP_MESSAGE = "Usage: " PROJECT_NAME APP_EXTENSION " [options]\n\n"
                                                 "Options:\n"
                                                 "\t--help, -h       Show this help message\n"
                                                 "\t--version, -v    Show version information\n"
                                                 "\t--config, -c     Specify path to config file\n";
static constexpr std::string_view VERSION_MESSAGE = PROJECT_NAME " version " PROJECT_VERSION "\n"
                                                                 "Build type: " BUILD_TYPE "\n"
                                                                 "Git tag: " GIT_TAG "\n"
                                                                 "Git commit hash: " GIT_COMMIT_HASH "\n";

rtp::ArgsConfig rtp::ArgsConfig::fromFile(const std::string &path)
{
    ArgsConfig cfg;
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    json j;
    file >> j;

    if (j.contains("window"))
    {
        const auto &w = j["window"];
        if (w.contains("width"))
        {
            cfg.width = w["width"];
        }
        if (w.contains("height"))
        {
            cfg.height = w["height"];
        }
        if (w.contains("frame_limit"))
        {
            cfg.frameLimit = w["frame_limit"];
        }
        if (w.contains("fullscreen"))
        {
            cfg.fullscreen = w["fullscreen"];
        }
    }
    return cfg;
}

rtp::ArgsConfig rtp::ArgsHandler::ParseArgs(const int argc, const char *const argv[])
{
    if (argc <= 1)
        return {};

    using ArgHandler = std::function<void(const char *arg)>;
    std::unordered_map<std::string_view, ArgHandler> handlers;
    ArgsConfig config{};
    for (const auto *const opt : {"-h", "--help"})
    {
        handlers[opt] = [&config](const char *)
        {
            std::cout << HELP_MESSAGE;
            config.exit = true;
        };
    }
    for (const auto *const opt : {"-v", "--version"})
    {
        handlers[opt] = [&config](const char *)
        {
            std::cout << VERSION_MESSAGE;
            config.exit = true;
        };
    }

    for (const auto *const opt : {"-c", "--config"})
    {
        handlers[opt] = [&config](const char *arg)
        {
            if (!arg)
            {
                throw std::runtime_error("Missing config file argument");
            }
            config = ArgsConfig::fromFile(arg);
            utl::Logger::log("Loaded config from file: " + std::string(arg), utl::LogLevel::INFO);
            std::cout << "\tWidth: " << config.width << '\n'
                      << "\tHeight: " << config.height << '\n'
                      << "\tFrameLimit: " << config.frameLimit << "\n"
                      << "\tFullscreen: " << (config.fullscreen ? "true" : "false") << '\n';
        };
    }

    const std::string_view key = argv[1];
    const char *argValue = (argc > 2) ? argv[2] : nullptr;

    if (const auto it = handlers.find(key); it != handlers.end())
    {
        it->second(argValue);
        return config;
    }

    throw std::runtime_error("Unknown argument: " + std::string(key));
}

rtp::EnvConfig rtp::ArgsHandler::ParseEnv(const char *const env[])
{
    (void)env; // Currently unused
    return {};
}