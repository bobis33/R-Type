#include <thread>

#include "Server/ArgsHandler.hpp"
#include "Server/Generated/Version.hpp"
#include "Server/Server.hpp"
#include "Utils/Logger.hpp"

srv::Server::Server(const ArgsConfig &config)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
                 "\tVersion: " PROJECT_VERSION "\n"
                 "\tBuild type: " BUILD_TYPE "\n"
                 "\tGit tag: " GIT_TAG "\n"
                 "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_pluginLoader = std::make_unique<utl::PluginLoader>();
    const std::filesystem::path asioNetworkPath =
        std::filesystem::path(PLUGINS_DIR) / ("network_asio_client" + std::string(PLUGINS_EXTENSION));
    m_network = m_pluginLoader->loadPlugin<INetworkServer>(asioNetworkPath);
    m_network->init(config.port, config.host);
}

void srv::Server::run() const
{
    m_network->start();
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}