#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Game.hpp"
#include "Client/Scenes/Lobby.hpp"
#include "Client/Scenes/Settings.hpp"
#include "Client/Systems/Systems.hpp"
#include "R-TypeClient/RTypeClient.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"
#include "Utils/PluginLoader.hpp"

static constexpr eng::Color DARK = {.r = 0U, .g = 0U, .b = 0U, .a = 255U};

cli::Client::Client(const ArgsConfig &cfg)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
              << "\tVersion: " PROJECT_VERSION "\n"
              << "\tBuild type: " BUILD_TYPE "\n"
              << "\tGit tag: " GIT_TAG "\n"
              << "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_pluginLoader = std::make_unique<utl::PluginLoader>();
    m_engine = std::make_unique<eng::Engine>(
        [this, cfg]() { return m_pluginLoader->loadPlugin<eng::IAudio>(cfg.audio_lib_path); },
        [this, cfg]() { return m_pluginLoader->loadPlugin<eng::INetworkClient>(cfg.network_lib_path); },
        [this, cfg]() { return m_pluginLoader->loadPlugin<eng::IRenderer>(cfg.renderer_lib_path); });
    // m_game = std::make_unique<gme::RTypeClient>();
    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);

    m_engine->addSystem(std::make_unique<AnimationSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<AudioSystem>(*m_engine->getAudio()));
    // m_engine->addSystem(std::make_unique<SpawnSystem>(*m_engine->getRenderer())); TODO(bobis33): only in game
    m_engine->addSystem(std::make_unique<AsteroidSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<BeamSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<CollisionSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<EnemySystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<ExplosionSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<LoadingAnimationSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<PixelSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<PlayerDirectionSystem>());
    m_engine->addSystem(std::make_unique<ProjectileSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<SpawnSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<SpriteSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<TextSystem>(*m_engine->getRenderer()));
    // m_engine->addSystem(std::make_unique<WeaponSystem>(*m_engine->getRenderer())); TODO(bobis33): tofix

    auto lobby = std::make_unique<Lobby>(m_engine->getRenderer(), m_engine->getAudio());
    auto game = std::make_unique<Game>(m_engine->getRenderer(), m_engine->getAudio());
    auto settings = std::make_unique<Settings>(m_engine->getRenderer(), m_engine->getAudio());
    const auto lobbyId = lobby->getId();
    const auto gameId = game->getId();
    const auto settingsId = settings->getId();
    lobby->onOptionSelected = [this, gameId, settingsId](const std::string &option)
    {
        if (option == "Solo")
        {
            m_engine->getSceneManager()->switchToScene(gameId);
        }
        else if (option == "Multi")
        {
            m_engine->getSceneManager()->switchToScene(gameId);
        }
        else if (option == "Settings")
        {
            m_engine->getSceneManager()->switchToScene(settingsId);
        }
    };
    settings->onLeave = [this, lobbyId]() { m_engine->getSceneManager()->switchToScene(lobbyId); };
    m_engine->getSceneManager()->addScene(std::move(lobby));
    m_engine->getSceneManager()->addScene(std::move(game));
    m_engine->getSceneManager()->addScene(std::move(settings));
    m_engine->getSceneManager()->switchToScene(lobbyId);
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
    m_engine->stop();
}
