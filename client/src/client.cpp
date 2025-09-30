#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Menu.hpp"
#include "Client/Scenes/Game.hpp"
#include "Client/Systems/Systems.hpp"
#include "SFMLAudio/SFMLAudio.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"
#include "Client/Scenes/Lobby.hpp"

static constexpr eng::Color DARK = {.r = 0U, .g = 0U, .b = 0U, .a = 255U};

cli::Client::Client(const ArgsConfig &cfg)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);

    m_engine = std::make_unique<eng::Engine>(
        [] { return std::make_unique<eng::SFMLAudio>(); },
        [] { return nullptr; },
        [] { return std::make_unique<eng::SFMLRenderer>(); });

    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);

    m_engine->getRenderer()->createFont("main_font", Path::Font::FONTS_RTYPE);

    m_engine->getAudio()->createAudio(Path::Audio::AUDIO_TITLE, 50.F, true, "title_music");
    m_engine->getAudio()->playAudio("title_music");

    m_engine->addSystem(std::make_unique<cli::TextSyStem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<cli::AudioSystem>(*m_engine->getAudio()));
    m_engine->addSystem(std::make_unique<cli::SpriteSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<cli::PixelSystem>(*m_engine->getRenderer()));

    auto menu = std::make_unique<Menu>(m_engine->getRenderer(), m_engine->getAudio());
    const auto menuId = menu->getId();
    m_engine->getSceneManager()->addScene(std::move(menu));
    m_engine->getSceneManager()->switchToScene(menuId);
}

void cli::Client::run()
{
    while (m_engine->getState() == eng::State::RUN && m_engine->getRenderer()->windowIsOpen())
    {
        const float delta = m_engine->getClock()->getDeltaSeconds();
        m_engine->getClock()->restart();

        auto &scene = *m_engine->getSceneManager()->getCurrentScene();
        auto *menu = dynamic_cast<Menu*>(&scene);

        eng::Event event;
        while (m_engine->getRenderer()->pollEvent(event))
        {
            if (event.type == eng::EventType::Closed) {
                m_engine->getRenderer()->closeWindow();
            } else {
                scene.event(event);
            }
        }

        scene.update(delta, m_engine->getRenderer()->getWindowSize());

        if (menu) {
            if (menu->shouldExitGame()) {
                m_engine->getRenderer()->closeWindow();
            }
            if (menu->shouldStartSolo()) {
                std::cout << "Switching to SOLO" << std::endl;
                auto gameScene = std::make_unique<cli::Game>(m_engine->getRenderer(), m_engine->getAudio());
                const auto gameId = gameScene->getId();
                m_engine->getSceneManager()->addScene(std::move(gameScene));
                m_engine->getSceneManager()->switchToScene(gameId);
            }
            if (menu->shouldStartMulti()) {
                std::cout << "Switching to MULTIPLAYER" << std::endl;
                auto lobbyScene = std::make_unique<cli::Lobby>(m_engine->getRenderer(), m_engine->getAudio());
                const auto lobbyId = lobbyScene->getId();
                m_engine->getSceneManager()->addScene(std::move(lobbyScene));
                m_engine->getSceneManager()->switchToScene(lobbyId);
            }
        }

        m_engine->render(scene.getRegistry(), DARK, delta);
    }
    m_engine->stop();
}
