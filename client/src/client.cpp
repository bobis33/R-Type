#include "Client/Client.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Systems/Systems.hpp"
#include "ECS/Component.hpp"
#include "R-TypeClient/RTypeClient.hpp"
#include "SFMLAudio/SFMLAudio.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"

static constexpr eng::Color DARK = {0U, 0U, 0U, 255U};

cli::Client::Client(const ArgsConfig &cfg)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
              << "\tVersion: " PROJECT_VERSION "\n"
              << "\tBuild type: " BUILD_TYPE "\n"
              << "\tGit tag: " GIT_TAG "\n"
              << "\tGit commit hash: " GIT_COMMIT_HASH "\n";

    m_engine =
        std::make_unique<eng::Engine>([]() { return std::make_unique<eng::SFMLAudio>(); }, []() { return nullptr; },
                                      []() { return std::make_unique<eng::SFMLRenderer>(); });
    m_game = std::make_unique<gme::RTypeClient>();

    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);

    m_engine->addSystem(std::make_unique<AudioSystem>(*m_engine->getAudio()));
    m_engine->addSystem(std::make_unique<PixelSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<SpriteSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<TextSyStem>(*m_engine->getRenderer()));

    const auto &lobby = lobbyScene();
    m_engine->getSceneManager()->switchToScene(lobby.getId());
}

void cli::Client::update(const float dt, const std::unique_ptr<eng::IScene> &scene)
{
    scene->onUpdate(dt);
}

void cli::Client::run()
{
    eng::Event event;

    while (m_engine->getState() == eng::State::RUN && m_engine->getRenderer()->windowIsOpen())
    {
        const float dt = m_engine->getClock()->getDeltaSeconds();

        m_engine->getClock()->restart();
        update(dt, m_engine->getSceneManager()->getCurrentScene());
        handleEvents(event);
        m_engine->render(m_engine->getSceneManager()->getCurrentScene()->getRegistry(), DARK, dt);
    }
    m_engine->stop();
}
