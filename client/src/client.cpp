#include "Client/Client.hpp"
#include "Client/Common.hpp"
#include "Client/Generated/Version.hpp"
#include "Client/Systems/Systems.hpp"
#include "ECS/Component.hpp"
#include "R-TypeClient/RTypeClient.hpp"
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
    m_engine =
        std::make_unique<eng::Engine>([]() { return std::make_unique<eng::SFMLAudio>(); }, []() { return nullptr; },
                                      []() { return std::make_unique<eng::SFMLRenderer>(); });
    m_game = std::make_unique<gme::RTypeClient>();
    m_engine->getRenderer()->createWindow("R-Type Client", cfg.height, cfg.width, cfg.frameLimit, cfg.fullscreen);
    m_engine->getRenderer()->createFont(eng::Font{.path = Paths::Fonts::FONTS_RTYPE, .name = "main_font"});
    m_engine->getAudio()->createAudio(Paths::Audio::AUDIO_TITLE, 5.F, true, "title_music");
    m_engine->getAudio()->playAudio("title_music");
    m_engine->addSystem(std::make_unique<AudioSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<PointSystem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<TextSyStem>(*m_engine->getRenderer()));
    m_engine->addSystem(std::make_unique<SpriteSystem>(*m_engine->getRenderer()));

    m_engine->getRegistry()->onComponentAdded( // TODO(bobis33): to refactor; no logic, not optimized
        [this](const ecs::Entity e, const std::type_info &type)
        {
            if (type == typeid(ecs::Text))
            {
                const auto *textComp = m_engine->getRegistry()->getComponent<ecs::Text>(e);
                const auto *transform = m_engine->getRegistry()->getComponent<ecs::Transform>(e);
                const auto *colorComp = m_engine->getRegistry()->getComponent<ecs::Color>(e);

                if (textComp && transform)
                {
                    const eng::Color color =
                        colorComp
                            ? eng::Color{.r = colorComp->r, .g = colorComp->g, .b = colorComp->b, .a = colorComp->a}
                            : eng::Color{.r = 255u, .g = 255u, .b = 255u, .a = 255u};

                    m_engine->getRenderer()->createText(eng::Text{.fontName = "main_font",
                                                                  .color = color,
                                                                  .content = textComp->content,
                                                                  .size = textComp->fontSize,
                                                                  .x = transform->x,
                                                                  .y = transform->y,
                                                                  .name = textComp->id});
                }
            }
            else if (type == typeid(ecs::Texture))
            {
                float scale_x = 0;
                float scale_y = 0;
                const auto *spriteComp = m_engine->getRegistry()->getComponent<ecs::Texture>(e);
                const auto *rectComp = m_engine->getRegistry()->getComponent<ecs::Rect>(e);
                const auto *scaleComp = m_engine->getRegistry()->getComponent<ecs::Scale>(e);
                scaleComp ? (scale_x = scaleComp->x, scale_y = scaleComp->y) : (scale_x = 1, scale_y = 1);
                if (const auto *transform = m_engine->getRegistry()->getComponent<ecs::Transform>(e);
                    spriteComp && transform)
                {
                    if (rectComp)
                    {
                        m_engine->getRenderer()->createSprite(
                            spriteComp->path, transform->x, transform->y, spriteComp->id, scale_x, scale_y,
                            static_cast<int>(rectComp->pos_x), static_cast<int>(rectComp->pos_y), rectComp->size_x,
                            rectComp->size_y);
                    }
                    else
                    {
                        m_engine->getRenderer()->createSprite(spriteComp->path, transform->x, transform->y,
                                                              spriteComp->id);
                    }
                }
            }
        });

    const auto titleEntity = m_engine->getRegistry()->createEntity();
    const auto fpsEntity = m_engine->getRegistry()->createEntity();
    m_engine->addComponent<ecs::Transform>(*m_engine->getRegistry(), titleEntity,
                                           "entity_" + std::to_string(titleEntity), 10.F, 10.F, 0.F);
    m_engine->addComponent<ecs::Color>(*m_engine->getRegistry(), titleEntity, "entity_" + std::to_string(titleEntity),
                                       static_cast<unsigned char>(255U), static_cast<unsigned char>(255U), static_cast<unsigned char>(255U), static_cast<unsigned char>(255U));
    m_engine->addComponent<ecs::Text>(*m_engine->getRegistry(), titleEntity, "entity_" + std::to_string(titleEntity),
                                      std::string("RType Client"), 50U);
    m_engine->addComponent<ecs::Transform>(*m_engine->getRegistry(), fpsEntity, "entity_" + std::to_string(fpsEntity),
                                           10.F, 70.F, 0.F);
    m_engine->addComponent<ecs::Color>(*m_engine->getRegistry(), fpsEntity, "entity_" + std::to_string(fpsEntity), static_cast<unsigned char>(255U),
                                       static_cast<unsigned char>(255U), static_cast<unsigned char>(255U), static_cast<unsigned char>(255U));
    m_engine->addComponent<ecs::Text>(*m_engine->getRegistry(), fpsEntity, "entity_" + std::to_string(fpsEntity),
                                      std::string("FPS 0"), 20U);
    m_fpsEntity = fpsEntity;
}

void cli::Client::run()
{
    eng::Event event;

    while (m_engine->getRenderer()->windowIsOpen())
    {
        const float dt = m_engine->getClock()->getDeltaSeconds();
        m_engine->getClock()->restart();

        m_game->update(dt, m_engine->getRenderer()->getWindowSize().width,
                       m_engine->getRenderer()->getWindowSize().height);
        syncEntitiesToECS();

        while (m_engine->getRenderer()->pollEvent(event))
        {
            if (event.type == eng::EventType::Closed ||
                (event.type == eng::EventType::KeyPressed && event.key == eng::Key::Escape))
            {
                m_engine->stop();
            }
        }
        if (auto *fpsText = m_engine->getRegistry()->getComponent<ecs::Text>(m_fpsEntity))
        {
            fpsText->content = "FPS " + std::to_string(static_cast<int>(1 / dt));
        }

        m_engine->render({.r = 0u, .g = 0u, .b = 0u, .a = 255u}, dt);
    }
}

void cli::Client::syncEntitiesToECS()
{ // TODO(bobis33): to remove/refactor, better handle from game
    for (const auto &entities = m_game->getCurrentScene().getEntities();
         const auto &[type, pos_x, pos_y, v_x, v_y, scale_x, scale_y, r, g, b, a, texture_path, text_rect_x,
                      text_rect_y, text_rect_fx, text_rect_fy, id] : entities)
    {
        ecs::Entity ecsEntity = 0;

        if (auto it = m_entityMap.find(id); it == m_entityMap.end())
        {
            ecsEntity = m_engine->getRegistry()->createEntity();
            m_entityMap[id] = ecsEntity;

            m_engine->addComponent<ecs::Transform>(*m_engine->getRegistry(), ecsEntity,
                                                   "entity_" + std::to_string(ecsEntity), pos_x, pos_y, 0.F);

            m_engine->addComponent<ecs::Velocity>(*m_engine->getRegistry(), ecsEntity,
                                                  "entity_" + std::to_string(ecsEntity), v_x, v_y);

            if (type == "star")
            {
                m_engine->addComponent<ecs::Point>(*m_engine->getRegistry(), ecsEntity, id, pos_x, pos_y,
                                                   ecs::Color{.id = "color_" + id, .r = r, .g = g, .b = b, .a = a});
            }
            else
            {
                m_engine->addComponent<ecs::Scale>(*m_engine->getRegistry(), ecsEntity,
                                                   "entity_" + std::to_string(ecsEntity), scale_x, scale_y);
                m_engine->addComponent<ecs::Rect>(*m_engine->getRegistry(), ecsEntity,
                                                  "entity_" + std::to_string(ecsEntity), text_rect_x, text_rect_y,
                                                  text_rect_fx, text_rect_fy);
                m_engine->addComponent<ecs::Transform>(*m_engine->getRegistry(), ecsEntity,
                                                       "entity_" + std::to_string(ecsEntity), pos_x, pos_y, 0.F);
                m_engine->addComponent<ecs::Texture>(*m_engine->getRegistry(), ecsEntity,
                                                     "entity_" + std::to_string(ecsEntity), texture_path);
            }
        }
        else
        {
            ecsEntity = it->second;

            if (auto *transform = m_engine->getRegistry()->getComponent<ecs::Transform>(ecsEntity))
            {
                transform->x = pos_x;
                transform->y = pos_y;
            }
            if (auto *velocity = m_engine->getRegistry()->getComponent<ecs::Velocity>(ecsEntity))
            {
                velocity->x = v_x;
                velocity->y = v_y;
            }

            if (type == "star")
            {
                if (auto *point = m_engine->getRegistry()->getComponent<ecs::Point>(ecsEntity))
                {
                    point->x = pos_x;
                    point->y = pos_y;
                }
            }
        }
    }
}
