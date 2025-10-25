#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Menu.hpp"
#include "Client/Scenes/ServerScene.hpp"
#include "Client/Scenes/Settings.hpp"
#include "Client/Scenes/game/multi/ConfigMulti.hpp"
#include "Client/Systems/Systems.hpp"
#include "Utils/Logger.hpp"
#include "Utils/PluginLoader.hpp"

static constexpr eng::Color DARK = {.r = 0U, .g = 0U, .b = 0U, .a = 255U};

static std::vector<cli::RoomInfo> g_availableRooms; // TODO: Replace with server-side room management
static int g_nextRoomId = 1;                        // TODO: Replace with server-side room ID generation

cli::AppConfig cli::Client::setupConfig(const ArgsConfig &cfg)
{
    AppConfig appConfig;

    appConfig.frameLimit = cfg.frameLimit;
    appConfig.fullscreen = cfg.fullscreen;
    appConfig.height = cfg.height;
    appConfig.width = cfg.width;
    appConfig.host = cfg.host;
    appConfig.port = cfg.port;

    appConfig.audioVolume = Config::Audio::DEFAULT_AUDIO_VOLUME;
    appConfig.videoQuality = Config::Game::DEFAULT_VIDEO_QUALITY;
    appConfig.controlScheme = Config::Game::DEFAULT_CONTROL_SCHEME;
    appConfig.skinIndex = Config::Game::DEFAULT_SKIN_INDEX;

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
                !cfg.audio_lib_path.empty() ? cfg.audio_lib_path : Path::Plugin::PLUGIN_AUDIO_SFML.string());
        },
        [this, cfg]()
        {
            return m_pluginLoader->loadPlugin<eng::INetworkClient>(
                !cfg.network_lib_path.empty() ? cfg.network_lib_path
                                              : Path::Plugin::PLUGIN_NETWORK_ASIO_CLIENT.string());
        },
        [this, cfg]()
        {
            return m_pluginLoader->loadPlugin<eng::IRenderer>(
                !cfg.renderer_lib_path.empty() ? cfg.renderer_lib_path : Path::Plugin::PLUGIN_RENDERER_SFML.string());
        });
    m_gameSolo = m_pluginLoader->loadPlugin<gme::IGameClient>(
        !cfg.game_solo_lib_path.empty() ? cfg.game_solo_lib_path : Path::Plugin::PLUGIN_GAME_SOLO.string());
    m_gameMulti = m_pluginLoader->loadPlugin<gme::IGameClient>(
        !cfg.game_multi_lib_path.empty() ? cfg.game_multi_lib_path : Path::Plugin::PLUGIN_GAME_MULTI.string());
    m_engine->getRenderer()->createWindow("R-Type Client", m_config.height, m_config.width, m_config.frameLimit,
                                          m_config.fullscreen);
}

void cli::Client::run()
{
    setupScenes();

    eng::Event event;
    while (m_engine->getState() == eng::State::RUN)
    {
        handleEvents(event);
        m_engine->render(m_engine->getRenderer()->getWindowSize(), DARK, m_showDebug);
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
    auto menu = std::make_unique<Menu>(menuId, m_engine->getRenderer(), m_engine->getAudio());
    menu->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio(), m_config));
    menu->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    menu->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    menu->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    menu->addSystem(std::make_unique<DebugSystem>(m_engine->getRenderer(), m_showDebug));

    m_gameSolo->init(*m_engine, m_config.audioVolume, m_config.skinIndex, m_showDebug, menuId);
    m_gameMulti->init(*m_engine, m_config.audioVolume, m_config.skinIndex, m_showDebug, menuId);

    auto serverSceneId = m_engine->getSceneManager()->generateNextId();
    auto serverScene = std::make_unique<ServerScene>(serverSceneId, m_engine->getRenderer(), m_engine->getAudio());
    serverScene->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio(), m_config));
    serverScene->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    serverScene->addSystem(std::make_unique<DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto configMultiId = m_engine->getSceneManager()->generateNextId();
    auto configMulti = std::make_unique<ConfigMulti>(configMultiId, m_engine->getRenderer(), m_engine->getAudio());
    configMulti->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio(), m_config));
    configMulti->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    configMulti->addSystem(std::make_unique<DebugSystem>(m_engine->getRenderer(), m_showDebug));
    auto settingsId = m_engine->getSceneManager()->generateNextId();
    auto settings = std::make_unique<Settings>(settingsId, m_engine->getRenderer(), m_engine->getAudio(), m_config);
    settings->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio(), m_config));
    settings->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    settings->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    settings->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    settings->addSystem(std::make_unique<DebugSystem>(m_engine->getRenderer(), m_showDebug));
    const auto configSoloId = m_gameSolo->getMainSceneId();
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

    serverScene->onConnect =
        [this, configMultiId](const std::string &playerName, const std::string &serverIP, const std::string &serverPort)
    { m_engine->getSceneManager()->switchToScene(configMultiId); };
    serverScene->onBackToMenu = [this, menuId]() { m_engine->getSceneManager()->switchToScene(menuId); };

    configMulti->onOptionSelected =
        [this, menuId, createRoomSceneId, joinRoomSceneId, joinRoomScenePtr](const std::string &option)
    {
        if (option == "Create room")
        {
            m_engine->getSceneManager()->switchToScene(createRoomSceneId);
        }
        else if (option == "Join room")
        {
            joinRoomScenePtr->setRooms(g_availableRooms);
            m_engine->getSceneManager()->switchToScene(joinRoomSceneId);
        }
        else if (option == "Go back to menu")
        {
            m_engine->getSceneManager()->switchToScene(menuId);
        }
    };
    settings->onLeave = [this, menuId]() { m_engine->getSceneManager()->switchToScene(menuId); };

    m_engine->getSceneManager()->addScene(std::move(menu));
    m_engine->getSceneManager()->addScene(std::move(serverScene));
    m_engine->getSceneManager()->addScene(std::move(configMulti));
    m_engine->getSceneManager()->addScene(std::move(settings));
    m_engine->getSceneManager()->switchToScene(menuId);
}
