#include "Client/Client.hpp"
#include "Client/Common.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Menu.hpp"
#include "Client/Game.hpp"
#include "ECS/Component.hpp"
#include "SFMLAudio/SFMLAudio.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"

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

    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);
    m_engine->getRenderer()->createFont(eng::Font{.path = Paths::Fonts::FONTS_RTYPE, .name = "main_font"});

    m_engine->getAudio()->createAudio(Paths::Audio::AUDIO_TITLE, 50.F, true, "title_music");
    m_engine->getAudio()->playAudio("title_music");

    m_engine->addSystem(std::make_unique<eng::TextSyStem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<eng::FontSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<eng::AudioSystem>(*m_engine->getRenderer()));

    m_engine->getRegistry()->onComponentAdded(
        [this](const ecs::Entity e, const std::type_info &type)
        {
            if (type == typeid(ecs::Text))
            {
                const auto *textComp = m_engine->getRegistry()->getComponent<ecs::Text>(e);
                if (const auto *transform = m_engine->getRegistry()->getComponent<ecs::Transform>(e);
                    textComp && transform)
                {
                    eng::Color col = {255, 255, 255, 255};
                    if (const auto *colorComp = m_engine->getRegistry()->getComponent<ecs::Color>(e); colorComp)
                    {
                        col = {.r = static_cast<std::uint8_t>(colorComp->r),
                               .g = static_cast<std::uint8_t>(colorComp->g),
                               .b = static_cast<std::uint8_t>(colorComp->b),
                               .a = static_cast<std::uint8_t>(colorComp->a)};
                    }

                    m_engine->getRenderer()->createText(
                        eng::Text{.fontName = "main_font",
                                  .color = col,
                                  .content = textComp->content,
                                  .size = textComp->fontSize,
                                  .x = static_cast<int>(transform->x),
                                  .y = static_cast<int>(transform->y),
                                  .name = textComp->id});
                }
            }
        });

    eng::Event event;
    bool inMenu = true;
    cli::Menu menu(*m_engine);
    std::unique_ptr<cli::Game> game = nullptr;

    while (m_engine->getRenderer()->windowIsOpen())
    {
        const float dt = m_engine->getClock()->getDeltaSeconds();
        m_engine->getClock()->restart();

        while (m_engine->getRenderer()->pollEvent(event))
        {
            if (event.type == eng::EventType::Closed)
            {
                m_engine->getRenderer()->closeWindow();
            }
            else if (inMenu)
            {
                menu.handleEvent(event);
            }
            else if (game)
            {
                game->handleEvent(event);
            }
        }

        // Logique menu / jeu
        if (inMenu)
        {
            if (menu.shouldExitGame())
            {
                m_engine->getRenderer()->closeWindow();
            }
            if (menu.shouldStartGame())
            {
                std::cout << "Switching to Game..." << std::endl;
                menu.unload();
                inMenu = false;
                game = std::make_unique<cli::Game>(*m_engine);
            }
            menu.update(dt);
        }
        else if (game)
        {
            game->update(dt);
        }

        m_engine->getRenderer()->clearWindow({.r = 0, .g = 0, .b = 0, .a = 255});
        m_engine->updateSystems(dt);
        m_engine->getRenderer()->displayWindow();
    }
}
