#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Menu.hpp"
#include "Client/Scenes/Settings.hpp"
#include "Client/Scenes/game/multi/ConfigMulti.hpp"
#include "Client/Scenes/game/solo/ConfigSolo.hpp"
#include "Client/Scenes/game/solo/GameSolo.hpp"
#include "Client/Systems/Systems.hpp"
#include "R-TypeClient/RTypeClient.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"
#include "Utils/PluginLoader.hpp"

static constexpr eng::Color DARK = {.r = 0U, .g = 0U, .b = 0U, .a = 255U};

cli::AppConfig cli::Client::setupConfig(const ArgsConfig &cfg)
{
    AppConfig appConfig;

    appConfig.frameLimit = cfg.frameLimit;
    appConfig.fullscreen = cfg.fullscreen;
    appConfig.height = cfg.height;
    appConfig.width = cfg.width;
    appConfig.host = cfg.host;
    appConfig.port = cfg.port;

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
    // m_game = std::make_unique<gme::RTypeClient>();
    m_engine->getRenderer()->createWindow("R-Type Client", m_config.height, m_config.width, m_config.frameLimit, m_config.fullscreen);
    m_engine->getNetwork()->connect(m_config.host, m_config.port);
    m_engine->getNetwork()->sendConnect("Bobi");
    m_engine->addSystem(std::make_unique<AnimationSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<AudioSystem>(m_engine->getAudio()));
    m_engine->addSystem(std::make_unique<SpawnSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<AsteroidSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<BeamSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<CollisionSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<EnemySystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<ExplosionSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<LoadingAnimationSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<PixelSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<PlayerDirectionSystem>());
    m_engine->addSystem(std::make_unique<ProjectileSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<SpriteSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<TextSystem>(m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<WeaponSystem>(m_engine->getRenderer()));

    auto menu = std::make_unique<Menu>(m_engine->getRenderer(), m_engine->getAudio());
    auto configMulti = std::make_unique<ConfigMulti>(m_engine->getRenderer(), m_engine->getAudio());
    auto configSolo = std::make_unique<ConfigSolo>(m_engine->getRenderer(), m_engine->getAudio());
    auto gameSolo = std::make_unique<GameSolo>(m_engine->getRenderer(), m_engine->getAudio());
    auto settings = std::make_unique<Settings>(m_engine->getRenderer(), m_engine->getAudio());
    const auto menuId = menu->getId();
    const auto configMultiId = configMulti->getId();
    const auto configSoloId = configSolo->getId();
    const auto gameSoloId = gameSolo->getId();
    const auto settingsId = settings->getId();
    menu->onOptionSelected = [this, configSoloId, configMultiId, settingsId](const std::string &option)
    {
        if (option == "Solo")
        {
            m_engine->getSceneManager()->switchToScene(configSoloId);
        }
        else if (option == "Multi")
        {
            m_engine->getSceneManager()->switchToScene(configMultiId);
        }
        else if (option == "Settings")
        {
            m_engine->getSceneManager()->switchToScene(settingsId);
        }
    };
    configMulti->onOptionSelected = [this, menuId](const std::string &option)
    {
        if (option == "Create room")
        {
            // m_engine->getSceneManager()->switchToScene(createRoomId);
        }
        else if (option == "Join room")
        {
            // m_engine->getSceneManager()->switchToScene(joinRoomId);
        }
        else if (option == "Go back to menu")
        {
            m_engine->getSceneManager()->switchToScene(menuId);
        }
    };
    configSolo->onOptionSelected = [this, gameSoloId, menuId](const std::string &option)
    {
        if (option == "Level easy")
        {
            m_engine->getSceneManager()->switchToScene(gameSoloId);
        }
        else if (option == "Level medium")
        {
            m_engine->getSceneManager()->switchToScene(gameSoloId);
        }
        else if (option == "Go back to menu")
        {
            m_engine->getSceneManager()->switchToScene(menuId);
        }
    };
    settings->onLeave = [this, menuId]() { m_engine->getSceneManager()->switchToScene(menuId); };
    m_engine->getSceneManager()->addScene(std::move(menu));
    m_engine->getSceneManager()->addScene(std::move(configMulti));
    m_engine->getSceneManager()->addScene(std::move(configSolo));
    m_engine->getSceneManager()->addScene(std::move(gameSolo));
    m_engine->getSceneManager()->addScene(std::move(settings));
    m_engine->getSceneManager()->switchToScene(menuId);
}

void cli::Client::run()
{
    eng::Event event;

    while (m_engine->getState() == eng::State::RUN && m_engine->getRenderer()->windowIsOpen())
    {
        const float delta = m_engine->getClock()->getDeltaSeconds();

        m_engine->getClock()->restart();
        m_engine->getSceneManager()->getCurrentScene()->update(delta, m_engine->getRenderer()->getWindowSize());
        handleEvents(event);
        m_engine->render(m_engine->getSceneManager()->getCurrentScene()->getRegistry(), DARK, delta);
    }
}

void cli::Client::stop() const
{
    m_engine->getNetwork()->disconnect();
    m_engine->stop();
}