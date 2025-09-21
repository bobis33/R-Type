#include <algorithm>

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
    m_engine->addSystem(std::make_unique<AudioSystem>(*m_engine->getAudio()));
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
                            : eng::Color{.r = 255U, .g = 255U, .b = 255U, .a = 255U};

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
            else if (type == typeid(ecs::Audio))
            {
                const auto *audioComp = m_engine->getRegistry()->getComponent<ecs::Audio>(e);
                m_engine->getAudio()->createAudio(audioComp->path, audioComp->volume, audioComp->loop, audioComp->id);
            }
        });

    const auto titleEntity =
        m_engine->getRegistry()
            ->createEntity()
            .with<ecs::Transform>("transform_title", 10.F, 10.F, 0.F)
            .with<ecs::Color>("color_title", static_cast<unsigned char>(255U), static_cast<unsigned char>(255U),
                              static_cast<unsigned char>(255U), static_cast<unsigned char>(255U))
            .with<ecs::Text>("id", std::string("RType Client"), 50U)
            .build();
    const auto audioEntity = m_engine->getRegistry()
                                 ->createEntity()
                                 .with<ecs::Audio>("id_audio", Paths::Audio::AUDIO_TITLE, 5.F, true, true)
                                 .build();
    m_fpsEntity = m_engine->getRegistry()
                      ->createEntity()
                      .with<ecs::Transform>("transform_fps", 10.F, 70.F, 0.F)
                      .with<ecs::Color>("color_fps", static_cast<unsigned char>(255U), static_cast<unsigned char>(255U),
                                        static_cast<unsigned char>(255U), static_cast<unsigned char>(255U))
                      .with<ecs::Text>("id_text", std::string("RType Client"), 20U)
                      .build();
    m_playerEntity = m_engine->getRegistry()
                         ->createEntity()
                         .with<ecs::Transform>("player_transform", 200.F, 100.F, 0.F)
                         .with<ecs::Velocity>("player_velocity", 500.F, 500.F)
                         .with<ecs::Rect>("player_rect", 0.F, 0.F, 33, 20)
                         .with<ecs::Scale>("player_scale", 2.F, 2.F)
                         .with<ecs::Texture>("player_tex", "assets/sprites/r-typesheet42.gif")
                         .build();
    for (int i = 0; i < 100; i++)
    {
        auto star = m_engine->getRegistry()
                        ->createEntity()
                        .with<ecs::Point>("star_point_" + std::to_string(i), 0.F, 0.F)
                        .with<ecs::Velocity>("star_vel", -20.F - static_cast<float>(std::rand() % 30), 0.F)
                        .with<ecs::Color>("star_color", 100U, 100U, 200U, 255U)
                        .build();
    }
}

void cli::Client::handleEvents(eng::Event &event, const float dt)
{
    while (m_engine->getRenderer()->pollEvent(event))
    {
        switch (event.type)
        {
            case eng::EventType::Closed:
                m_engine->stop();
                break;

            case eng::EventType::KeyPressed:
                if (event.key == eng::Key::Escape)
                {
                    m_engine->stop();
                }
                else
                {
                    m_keysPressed[event.key] = true;
                }
                break;

            case eng::EventType::KeyReleased:
                m_keysPressed[event.key] = false;
                break;

            default:
                break;
        }
    }
}

void cli::Client::run()
{
    eng::Event event;

    while (m_engine->getRenderer()->windowIsOpen())
    {
        const float dt = m_engine->getClock()->getDeltaSeconds();
        m_engine->getClock()->restart();

        for (auto &[entity, velocity] : m_engine->getRegistry()->getAll<ecs::Velocity>())
        {
            if (auto *point = m_engine->getRegistry()->getComponent<ecs::Point>(entity))
            {
                point->x += velocity.x * dt;
                point->y += velocity.y * dt;

                if (point->x < 2.F || point->y < 2.F)
                {
                    point->x = static_cast<float>(std::rand() % m_engine->getRenderer()->getWindowSize().width);
                    point->y = static_cast<float>(std::rand() % m_engine->getRenderer()->getWindowSize().height);
                }
            }
        }
        if (auto *fpsText = m_engine->getRegistry()->getComponent<ecs::Text>(m_fpsEntity))
        {
            fpsText->content = "FPS " + std::to_string(static_cast<int>(1 / dt));
        }
        auto *playerTransform = m_engine->getRegistry()->getComponent<ecs::Transform>(m_playerEntity);
        const auto *playerVelocity = m_engine->getRegistry()->getComponent<ecs::Velocity>(m_playerEntity);
        // if ((playerTransform == nullptr) || (playerVelocity == nullptr)) {
        //     return;
        // }

        if (m_keysPressed[eng::Key::Up])
        {
            playerTransform->y -= playerVelocity->y * dt;
        }
        if (m_keysPressed[eng::Key::Down])
        {
            playerTransform->y += playerVelocity->y * dt;
        }
        if (m_keysPressed[eng::Key::Left])
        {
            playerTransform->x -= playerVelocity->x * dt;
        }
        if (m_keysPressed[eng::Key::Right])
        {
            playerTransform->x += playerVelocity->x * dt;
        }
        const auto [width, height] = m_engine->getRenderer()->getWindowSize();

        playerTransform->x = std::max(playerTransform->x, 0.f);
        playerTransform->y = std::max(playerTransform->y, 0.f);
        playerTransform->x = std::min(playerTransform->x, static_cast<float>(width) - 66.f);
        playerTransform->y = std::min(playerTransform->y, static_cast<float>(height) - 40.f);

        handleEvents(event, dt);
        m_engine->render({.r = 0U, .g = 0U, .b = 0U, .a = 255U}, dt);
    }
}
