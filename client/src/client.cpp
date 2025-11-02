#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Intro.hpp"
#include "Client/Scenes/Menu.hpp"
#include "Client/Scenes/Settings.hpp"
#include "Client/Scenes/WinCondition.hpp"
#include "ECS/Systems/Systems.hpp"
#include "Utils/Logger.hpp"
#include "Utils/PluginLoader.hpp"

utl::cli::AppConfig cli::Client::setupConfig(const ArgsConfig &cfg)
{
    utl::cli::AppConfig appConfig;

    appConfig.frameLimit = cfg.frameLimit;
    appConfig.fullscreen = cfg.fullscreen;
    appConfig.height = cfg.height;
    appConfig.width = cfg.width;
    appConfig.host = cfg.host;
    appConfig.port = cfg.port;
    appConfig.player_name = cfg.player_name;

    appConfig.audioVolume = utl::Config::Audio::DEFAULT_AUDIO_VOLUME;
    appConfig.videoQuality = utl::Config::Game::DEFAULT_VIDEO_QUALITY;
    appConfig.controlScheme = utl::Config::Game::DEFAULT_CONTROL_SCHEME;
    appConfig.skinIndex = utl::Config::Game::DEFAULT_SKIN_INDEX;

    return appConfig;
}

cli::Client::Client(const ArgsConfig &cfg)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
              << "\tVersion: " PROJECT_VERSION "\n"
              << "\tBuild type: " BUILD_TYPE "\n"
              << "\tGit tag: " GIT_TAG "\n"
              << "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_config = setupConfig(cfg);
    m_pluginLoader = std::make_unique<utl::PluginLoader>();
    m_engine = std::make_unique<eng::Engine>(
        [this, cfg]()
        {
            return m_pluginLoader->loadPlugin<eng::IAudio>(
                !cfg.audio_lib_path.empty() ? cfg.audio_lib_path : utl::Path::Plugin::PLUGIN_AUDIO_SFML.string());
        },
        [this, cfg]()
        {
            return m_pluginLoader->loadPlugin<eng::INetworkClient>(
                !cfg.network_lib_path.empty() ? cfg.network_lib_path
                                              : utl::Path::Plugin::PLUGIN_NETWORK_ASIO_CLIENT.string());
        },
        [this, cfg]()
        {
            return m_pluginLoader->loadPlugin<eng::IRenderer>(!cfg.renderer_lib_path.empty()
                                                                  ? cfg.renderer_lib_path
                                                                  : utl::Path::Plugin::PLUGIN_RENDERER_SFML.string());
        });
    m_gameSolo = m_pluginLoader->loadPlugin<gme::IGameClient>(
        !cfg.game_solo_lib_path.empty() ? cfg.game_solo_lib_path : utl::Path::Plugin::PLUGIN_GAME_SOLO.string());
    m_gameMulti = m_pluginLoader->loadPlugin<gme::IGameClient>(
        !cfg.game_multi_lib_path.empty() ? cfg.game_multi_lib_path : utl::Path::Plugin::PLUGIN_GAME_MULTI.string());
    m_engine->getRenderer()->createWindow("R-Type Client", m_config.height, m_config.width, m_config.frameLimit,
                                          m_config.fullscreen);
    m_engine->getRenderer()->setWindowIcon(utl::Path::Texture::ICON_PATH);
}

void cli::Client::run()
{
    setupScenes();

    eng::Event event;
    while (m_engine->getState() == eng::State::RUN)
    {
        handleEvents(event);
        m_engine->render(m_engine->getRenderer()->getWindowSize(), utl::Config::Color::DARK, m_showDebug);
        m_engine->getNetwork()->update();
        m_gameSolo->update(m_engine->getClock()->getDeltaSeconds(), m_engine->getRenderer()->getWindowSize().width,
                           m_engine->getRenderer()->getWindowSize().height);
    }
}

void cli::Client::stop() const
{
    m_engine->getNetwork()->disconnect();
    m_engine->stop();
}

void cli::Client::setupScenes()
{
    auto menuId = m_engine->getSceneManager()->generateNextId();
    auto menu = std::make_unique<Menu>(menuId, m_engine->getRenderer());
    menu->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_config.audioVolume, menu->getRegistry(),
                                                       menu->playMusic()));
    menu->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), menu->getRegistry()));
    menu->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    menu->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    menu->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));

    auto winConditionId = m_engine->getSceneManager()->generateNextId();
    auto winCondition = std::make_unique<WinCondition>(winConditionId, m_engine->getRenderer(), m_engine->getAudio());
    winCondition->addSystem(
        std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_config.audioVolume, winCondition->getRegistry(), winCondition->playMusic()));
    winCondition->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    winCondition->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    winCondition->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    winCondition->onLeave = [this, menuId]() { m_engine->getSceneManager()->switchToScene(menuId); };

    auto introId = m_engine->getSceneManager()->generateNextId();
    auto intro = std::make_unique<Intro>(introId, m_engine->getRenderer(), m_engine->getAudio());
    intro->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_config.audioVolume,
                                                        intro->getRegistry(), menu->playMusic()));
    intro->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), intro->getRegistry()));
    intro->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    intro->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    intro->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    intro->onLeave = [this, menuId]() { m_engine->getSceneManager()->switchToScene(menuId); };

    auto settingsId = m_engine->getSceneManager()->generateNextId();
    auto settings = std::make_unique<Settings>(settingsId, m_engine->getRenderer(), m_config);
    settings->addSystem(std::make_unique<ecs::AudioSystem>(m_engine->getAudio(), m_config.audioVolume,
                                                           settings->getRegistry(), settings->playMusic()));
    settings->addSystem(std::make_unique<ecs::StarfieldSystem>(m_engine->getRenderer(), settings->getRegistry()));
    settings->addSystem(std::make_unique<ecs::SpriteSystem>(m_engine->getRenderer()));
    settings->addSystem(std::make_unique<ecs::TextSystem>(m_engine->getRenderer()));
    settings->addSystem(std::make_unique<ecs::DebugSystem>(m_engine->getRenderer(), m_showDebug));
    const auto configSoloId = m_gameSolo->getMainSceneId();
    const auto serverSceneId = m_gameMulti->getMainSceneId();
    menu->onOptionSelected = [this, configSoloId, serverSceneId, settingsId](const std::string &option)
    {
        if (option == "Solo")
        {
            m_engine->getSceneManager()->switchToScene(configSoloId);
        }
        else if (option == "Multi")
        {
            m_engine->getSceneManager()->switchToScene(serverSceneId);
        }
        else if (option == "Settings")
        {
            m_engine->getSceneManager()->switchToScene(settingsId);
        }
    };
    settings->onLeave = [this, menuId]() { m_engine->getSceneManager()->switchToScene(menuId); };

    m_engine->getSceneManager()->addScene(std::move(menu));
    m_engine->getSceneManager()->addScene(std::move(intro));
    m_engine->getSceneManager()->addScene(std::move(settings));
    m_engine->getSceneManager()->addScene(std::move(winCondition));
    m_engine->getSceneManager()->switchToScene(introId);

    m_gameSolo->init(*m_engine, m_config, m_showDebug, menuId, winConditionId);
    m_gameMulti->init(*m_engine, m_config, m_showDebug, menuId, winConditionId);
}
