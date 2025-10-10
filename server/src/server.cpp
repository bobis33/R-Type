#include <thread>

#include "Server/ArgsHandler.hpp"
#include "Server/Common.hpp"
#include "Server/Generated/Version.hpp"
#include "Server/Server.hpp"
#include "Utils/Logger.hpp"

srv::Server::Server(const ArgsConfig &config)
    : m_pluginLoader(std::make_unique<utl::PluginLoader>()),
      m_network(m_pluginLoader->loadPlugin<INetworkServer>(!config.network_lib_path.empty()
                                                               ? config.network_lib_path
                                                               : Path::Plugin::PLUGINS_NETWORK_ASIO_SERVER.string()))
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
                 "\tVersion: " PROJECT_VERSION "\n"
                 "\tBuild type: " BUILD_TYPE "\n"
                 "\tGit tag: " GIT_TAG "\n"
                 "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_network->init(config.host, config.port);
}

void srv::Server::run() const
{
    m_network->start();
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}