#include "Client/Client.hpp"
#include "Client/Common.hpp"
#include "Client/Generated/Version.hpp"
#include "ECS/Component.hpp"
#include "SFMLAudio/SFMLAudio.hpp"
#include "SFMLRenderer/SFMLRenderer.hpp"
#include "Utils/Clock.hpp"
#include "Utils/Logger.hpp"
#include "Client/Systems/Systems.hpp"

cli::Client::Client(const ArgsConfig &cfg)
{
    utl::Logger::log("PROJECT INFO:", utl::LogLevel::INFO);
    std::cout << "\tName: " PROJECT_NAME "\n"
              << "\tVersion: " PROJECT_VERSION "\n"
              << "\tBuild type: " BUILD_TYPE "\n"
              << "\tGit tag: " GIT_TAG "\n"
              << "\tGit commit hash: " GIT_COMMIT_HASH "\n";
    m_engine =
        std::make_unique<eng::Engine>([]() { return std::make_unique<eng::SFMLAudio>(); },
                                    []() { return nullptr; },
                                    []() { return std::make_unique<eng::SFMLRenderer>(); });

    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);
    m_engine->getRenderer()->createFont(eng::Font{.path = Paths::Fonts::FONTS_RTYPE, .name = "main_font"});
    m_engine->getAudio()->createAudio(Paths::Audio::AUDIO_TITLE, 50.F, true, "title_music");
    m_engine->getAudio()->playAudio("title_music");
    m_engine->addSystem(std::make_unique<TextSyStem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<FontSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<AudioSystem>(*m_engine->getRenderer()));

    m_engine->getRegistry()->onComponentAdded(
        [this](const ecs::Entity e, const std::type_info &type)
        {
            if (type == typeid(ecs::Text))
            {
                const auto *textComp = m_engine->getRegistry()->getComponent<ecs::Text>(e);
                if (const auto *transform = m_engine->getRegistry()->getComponent<ecs::Transform>(e);
                    textComp && transform)
                {
                    if (const auto *colorComp = m_engine->getRegistry()->getComponent<ecs::Color>(e); colorComp)
                    {
                        m_engine->getRenderer()->createText(
                            eng::Text{.fontName = "main_font",
                                      .color = eng::Color{.r = static_cast<std::uint8_t>(colorComp->r),
                                                          .g = static_cast<std::uint8_t>(colorComp->g),
                                                          .b = static_cast<std::uint8_t>(colorComp->b),
                                                          .a = static_cast<std::uint8_t>(colorComp->a)},
                                      .content = textComp->content,
                                      .size = textComp->fontSize,
                                      .x = static_cast<int>(transform->x),
                                      .y = static_cast<int>(transform->y),
                                      .name = textComp->id});
                        return;
                    }
                    m_engine->getRenderer()->createText(
                        eng::Text{.fontName = "main_font",
                                  .color = eng::Color{.r = 255, .g = 255, .b = 255, .a = 255},
                                  .content = textComp->content,
                                  .size = textComp->fontSize,
                                  .x = static_cast<int>(transform->x),
                                  .y = static_cast<int>(transform->y),
                                  .name = textComp->id});
                }
            }
        });

    const auto titleEntity = m_engine->getRegistry()->createEntity();
    m_engine->addComponent<ecs::Transform>(*m_engine->getRegistry(), titleEntity, "entity_" + std::to_string(titleEntity), 10.F,
                                                          10.F, 0.F);
    m_engine->addComponent<ecs::Color>(*m_engine->getRegistry(), titleEntity, "entity_" + std::to_string(titleEntity), 255, 255, 255, 255);
    m_engine->addComponent<ecs::Text>(*m_engine->getRegistry(), titleEntity, "entity_" + std::to_string(titleEntity),
                                                     std::string("RType Client"), 50);

    const auto fpsEntity = m_engine->getRegistry()->createEntity();
    m_engine->addComponent<ecs::Transform>(*m_engine->getRegistry(), fpsEntity, "entity_" + std::to_string(fpsEntity), 10.F, 70.F,
                                                          0.F);
    m_engine->addComponent<ecs::Color>(*m_engine->getRegistry(), fpsEntity, "entity_" + std::to_string(fpsEntity), 255, 255, 255,
                                                      255);
    m_engine->addComponent<ecs::Text>(*m_engine->getRegistry(), fpsEntity, "entity_" + std::to_string(fpsEntity),
                                                     std::string("FPS 0"), 20);

    eng::Event event;
    while (m_engine->getRenderer()->windowIsOpen())
    {
        const float dt = m_engine->getClock()->getDeltaSeconds();
        m_engine->getClock()->restart();

        while (m_engine->getRenderer()->pollEvent(event))
        {
            if (event.type == eng::EventType::Closed ||
                (event.type == eng::EventType::KeyPressed && event.key == eng::Key::Escape))
            {
                m_engine->stop();
            }
        }
        if (auto *fpsText = m_engine->getRegistry()->getComponent<ecs::Text>(fpsEntity))
        {
            fpsText->content = "FPS " + std::to_string(static_cast<int>(1.F / dt));
        }

        m_engine->render({.r = 0, .g = 0, .b = 0, .a = 255}, dt);
    }
}
