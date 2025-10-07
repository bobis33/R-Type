#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Menu.hpp"
#include "Client/Scenes/Game.hpp"
#include "Client/Scenes/Lobby.hpp"
#include "Client/Scenes/Settings.hpp"
#include "Client/Scenes/Room.hpp"
#include "Client/Systems/Systems.hpp"
#include "SFMLAudio/SFMLAudio.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"

static constexpr eng::Color DARK = {.r = 0U, .g = 0U, .b = 0U, .a = 255U};

namespace cli
{
    Client::Client(const ArgsConfig &cfg)
    {
        utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
        std::cout << "\tName: " PROJECT_NAME "\n"
                  << "\tVersion: " PROJECT_VERSION "\n"
                  << "\tBuild type: " BUILD_TYPE "\n"
                  << "\tGit tag: " GIT_TAG "\n"
                  << "\tGit commit hash: " GIT_COMMIT_HASH "\n";

        m_engine = std::make_unique<eng::Engine>(
            [] { return std::make_unique<eng::SFMLAudio>(); },
            [] { return nullptr; },
            [] { return std::make_unique<eng::SFMLRenderer>(); });

        m_engine->getRenderer()->createWindow(
            "R-Type Client",
            cfg.height,
            cfg.width,
            cfg.frameLimit,
            cfg.fullscreen);

        m_engine->addSystem(std::make_unique<cli::TextSyStem>(*m_engine->getRenderer()));
        m_engine->addSystem(std::make_unique<cli::AudioSystem>(*m_engine->getAudio()));
        m_engine->addSystem(std::make_unique<cli::SpriteSystem>(*m_engine->getRenderer()));
        m_engine->addSystem(std::make_unique<cli::PixelSystem>(*m_engine->getRenderer()));

        auto &sceneMgr = *m_engine->getSceneManager();
        auto rendererPtr = std::shared_ptr<eng::IRenderer>(m_engine->getRenderer().get(), [](eng::IRenderer*) {});
        auto audioPtr = std::shared_ptr<eng::IAudio>(m_engine->getAudio().get(), [](eng::IAudio*) {});

        auto menu = std::make_unique<Menu>(rendererPtr, audioPtr, &sceneMgr);
        auto game = std::make_unique<Game>(rendererPtr, audioPtr, &sceneMgr);
        auto lobby = std::make_unique<Lobby>(rendererPtr, audioPtr, &sceneMgr);
        auto settings = std::make_unique<Settings>(rendererPtr, audioPtr, &sceneMgr);
        auto room = std::make_unique<Room>(rendererPtr, audioPtr, &sceneMgr, true);

        sceneMgr.addScene(std::move(menu));
        sceneMgr.addScene(std::move(game));
        sceneMgr.addScene(std::move(lobby));
        sceneMgr.addScene(std::move(settings));
        sceneMgr.addScene(std::move(room));

        sceneMgr.switchToScene(1);
    }

    void Client::run()
    {
        eng::Event event;
        auto &renderer = *m_engine->getRenderer();
        auto &sceneMgr = *m_engine->getSceneManager();

        while (m_engine->getState() == eng::State::RUN && renderer.windowIsOpen())
        {
            const float dt = m_engine->getClock()->getDeltaSeconds();
            m_engine->getClock()->restart();

            auto &scene = *sceneMgr.getCurrentScene();

            while (renderer.pollEvent(event))
            {
                if (event.type == eng::EventType::Closed)
                    renderer.closeWindow();
                else
                    scene.event(event);
            }
            scene.update(dt, renderer.getWindowSize());
            m_engine->render(scene.getRegistry(), DARK, dt);
        }

        m_engine->stop();
    }
} // namespace cli
