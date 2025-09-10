#include "R-Type/Client.hpp"
#include "R-Type/Generated/Version.hpp"
#include "Utils/Logger.hpp"

rtp::Client::Client(const ArgsConfig &cfg) : m_renderer(cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
                 "\tVersion: " PROJECT_VERSION "\n"
                 "\tBuild type: " BUILD_TYPE "\n"
                 "\tGit tag: " GIT_TAG "\n"
                 "\tGit commit hash: " GIT_COMMIT_HASH "\n";
    m_renderer.run();
}