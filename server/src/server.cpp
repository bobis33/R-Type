#include <thread>

#include "Server/ArgsHandler.hpp"
#include "Server/Common.hpp"
#include "Server/Generated/Version.hpp"
#include "Server/Server.hpp"
#include "Utils/Logger.hpp"

srv::Server::Server(const ArgsConfig &config)
    : m_pluginLoader(std::make_unique<utl::PluginLoader>()), m_clock(std::make_unique<utl::Clock>()),
      m_sceneManager(std::make_unique<SceneManager>()),
      m_network(m_pluginLoader->loadPlugin<INetworkServer>(!config.network_lib_path.empty()
                                                               ? config.network_lib_path
                                                               : Path::Plugin::PLUGINS_NETWORK_ASIO_SERVER.string())),
      m_game(m_pluginLoader->loadPlugin<gme::IGameServer>(
          !config.game_lib_path.empty() ? config.game_lib_path : Path::Plugin::PLUGINS_GAME_RTYPE_SERVER.string()))
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
                 "\tVersion: " PROJECT_VERSION "\n"
                 "\tBuild type: " BUILD_TYPE "\n"
                 "\tGit tag: " GIT_TAG "\n"
                 "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_config = setupConfig(config);
    m_network->init(config.host, config.port);
}

void srv::Server::run() const
{
    m_network->start();
    m_game->start();

    auto startTime = std::chrono::steady_clock::now();

    for (;;)
    {
        auto currentTime = std::chrono::steady_clock::now();
        const float deltaTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
        startTime = currentTime;

        m_network->update();
        m_game->update(deltaTime);

        // Print connected sessions
        auto sessions = m_network->getConnectedSessions();
        std::cout << "Connected sessions (" << sessions.size() << "): ";
        for (size_t i = 0; i < sessions.size(); ++i)
        {
            std::cout << sessions[i];
            if (i < sessions.size() - 1) {
                std::cout << ", ";
}
        }
        std::cout << '\n';

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

utl::srv::AppConfig srv::Server::setupConfig(const ArgsConfig &cfg)
{
    utl::srv::AppConfig config;

    config.host = cfg.host;
    config.port = cfg.port;

    return config;
}
