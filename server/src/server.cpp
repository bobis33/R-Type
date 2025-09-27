#include "Server/Server.hpp"
#include "AsioServer/AsioServer.hpp"
#include "Server/ArgsHandler.hpp"
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

    m_network = std::make_unique<AsioServer>(config.port, config.host);
}

void srv::Server::run() const
{
    m_network->start();
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}