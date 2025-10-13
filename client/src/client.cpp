#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Systems/Systems.hpp"
#include "Interfaces/IGameClient.hpp"
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
    
    // Load game plugin (Local by default, can be changed to PLUGIN_GAME_RTYPE_MULTI for multiplayer)
    m_game = m_pluginLoader->loadPlugin<gme::IGameClient>(Path::Plugin::PLUGIN_GAME_RTYPE_LOCAL.string());
    
    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);

    // Initialize systems
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
    m_engine->addSystem(std::make_unique<SpriteSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<TextSystem>(*m_engine->getRenderer()));
    // m_engine->addSystem(std::make_unique<WeaponSystem>(*m_engine->getRenderer())); TODO(bobis33): tofix

    // Initialize game plugin with renderer and audio
    if (m_game)
    {
        m_game->initialize(m_engine->getRenderer(), m_engine->getAudio());
    }
}

void cli::Client::run()
{
    eng::Event event;

    while (m_engine->getState() == eng::State::RUN && m_engine->getRenderer()->windowIsOpen())
    {
        const float delta = m_engine->getClock()->getDeltaSeconds();

        m_engine->getClock()->restart();
        
        // Update game via plugin
        if (m_game)
        {
            const auto windowSize = m_engine->getRenderer()->getWindowSize();
            m_game->update(delta, windowSize.width, windowSize.height);
            
            // Render current scene from game plugin
            if (auto *currentScene = m_game->getCurrentEngineScene())
            {
                m_engine->render(currentScene->getRegistry(), DARK, delta);
            }
        }
        
        handleEvents(event);
    }
    m_engine->stop();
}
