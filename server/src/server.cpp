#include "Server/Server.hpp"
#include "Server/Generated/Version.hpp"
#include "Utils/Logger.hpp"

srv::Server::Server(const ArgsConfig &config)
{
    (void)config;
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
                 "\tVersion: " PROJECT_VERSION "\n"
                 "\tBuild type: " BUILD_TYPE "\n"
                 "\tGit tag: " GIT_TAG "\n"
                 "\tGit commit hash: " GIT_COMMIT_HASH "\n";
}