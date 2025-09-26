#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Scenes/Lobby.hpp"
#include "Client/Systems/Systems.hpp"
#include "R-TypeClient/RTypeClient.hpp"
#include "SFMLAudio/SFMLAudio.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"

static constexpr eng::Color DARK = {.r = 0U, .g = 0U, .b = 0U, .a = 255U};

cli::Client::Client(const ArgsConfig &cfg)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
              << "\tVersion: " PROJECT_VERSION "\n"
              << "\tBuild type: " BUILD_TYPE "\n"
              << "\tGit tag: " GIT_TAG "\n"
              << "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_engine = std::make_unique<eng::Engine>(
        []() { return std::make_unique<eng::SFMLAudio>(); },
        []() { return nullptr; },
        []() { return std::make_unique<eng::SFMLRenderer>(); });


    m_engine =
        std::make_unique<eng::Engine>([]() { return std::make_unique<eng::SFMLAudio>(); }, []() { return nullptr; },
                                      []() { return std::make_unique<eng::SFMLRenderer>(); });
    m_game = std::make_unique<gme::RTypeClient>();

    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);
    m_engine->getRenderer()->createFont(eng::Font{.path = Paths::Fonts::FONTS_RTYPE, .name = "main_font"});

    m_engine->getAudio()->createAudio(Paths::Audio::AUDIO_TITLE, 50.F, true, "title_music");
    m_engine->getAudio()->playAudio("title_music");

    m_engine->addSystem(std::make_unique<eng::TextSyStem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<eng::FontSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<eng::AudioSystem>(*m_engine->getRenderer()));

    m_engine->addSystem(std::make_unique<AudioSystem>(*m_engine->getAudio()));
    m_engine->addSystem(std::make_unique<PixelSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<SpriteSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<TextSyStem>(*m_engine->getRenderer()));

    auto lobby = std::make_unique<Lobby>(m_engine->getRenderer(), m_engine->getAudio());
    const auto lobbyId = lobby->getId();
    m_engine->getSceneManager()->addScene(std::move(lobby));
    m_engine->getSceneManager()->switchToScene(lobbyId);
}

void cli::Client::run()
{
    eng::Event event;
    bool inMenu = true;
    cli::Menu menu(*m_engine);
    std::unique_ptr<cli::Game> game = nullptr;

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
