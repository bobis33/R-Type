#include "Client/Client.hpp"
#include "Client/Common.hpp"
#include "ECS/Component.hpp"

static constexpr eng::Color WHITE = {255U, 255U, 255U, 255U};

eng::IScene& cli::Client::lobbyScene()
{
    return m_engine->getSceneManager()->createScene(
        "Lobby",
        [this](const eng::Event &event)
    {
        auto &reg = m_engine->getSceneManager()->getCurrentScene()->getRegistry();
        switch (event.type)
        {
            case eng::EventType::KeyPressed:
                if (event.key == eng::Key::Up)    m_keysPressed[eng::Key::Up] = true;
                if (event.key == eng::Key::Down)  m_keysPressed[eng::Key::Down] = true;
                if (event.key == eng::Key::Left)  m_keysPressed[eng::Key::Left] = true;
                if (event.key == eng::Key::Right) m_keysPressed[eng::Key::Right] = true;
                break;

            case eng::EventType::KeyReleased:
                if (event.key == eng::Key::Up)    m_keysPressed[eng::Key::Up] = false;
                if (event.key == eng::Key::Down)  m_keysPressed[eng::Key::Down] = false;
                if (event.key == eng::Key::Left)  m_keysPressed[eng::Key::Left] = false;
                if (event.key == eng::Key::Right) m_keysPressed[eng::Key::Right] = false;
                break;

            default:
                break;
        }
    },
    [this](const float dt)
    {
    auto &reg = m_engine->getSceneManager()->getCurrentScene()->getRegistry();
    auto *playerTransform = reg.getComponent<ecs::Transform>(m_playerEntity);
    const auto *playerVelocity = reg.getComponent<ecs::Velocity>(m_playerEntity);
    for (auto &[entity, velocity] : reg.getAll<ecs::Velocity>())
    {
        if (auto *pixel = reg.getComponent<ecs::Pixel>(entity))
        {
            if (auto *transform = reg.getComponent<ecs::Transform>(entity))
            {
                transform->x += velocity.x * dt;
                transform->y += velocity.y * dt;

                if (transform->x < 2.F || transform->y < 2.F)
                {
                    transform->x =
                        static_cast<float>(std::rand() % (m_engine->getRenderer()->getWindowSize().width * 2));
                    transform->y = static_cast<float>(std::rand() % m_engine->getRenderer()->getWindowSize().height);
                }
            }
        }
    }
    if (auto *fpsText = reg.getComponent<ecs::Text>(m_fpsEntity))
    {
        fpsText->content = "FPS " + std::to_string(static_cast<int>(1 / dt));
    }
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

    playerTransform->x = std::max(playerTransform->x, 0.F);
    playerTransform->y = std::max(playerTransform->y, 0.F);
    playerTransform->x = std::min(playerTransform->x, static_cast<float>(width) - 66.F); // TODO(bobis33): getTextureSize.x
    playerTransform->y = std::min(playerTransform->y, static_cast<float>(height) - 40.F); // TODO(bobis33): getTextureSize.y
    },
        [this](ecs::Registry &reg)
        {
            reg.onComponentAdded(
                [this, &reg](ecs::Entity e, const std::type_info &type)
                {
                    const auto *audioComp = reg.getComponent<ecs::Audio>(e);
                    const auto *colorComp = reg.getComponent<ecs::Color>(e);
                    const auto *fontComp = reg.getComponent<ecs::Font>(e);
                    const auto *rectComp = reg.getComponent<ecs::Rect>(e);
                    const auto *scaleComp = reg.getComponent<ecs::Scale>(e);
                    const auto *textComp = reg.getComponent<ecs::Text>(e);
                    const auto *textureComp = reg.getComponent<ecs::Texture>(e);
                    const auto *transform = reg.getComponent<ecs::Transform>(e);

                    if (type == typeid(ecs::Text))
                    {
                        if (textComp && transform && fontComp)
                        {
                            m_engine->getRenderer()->createFont({fontComp->path, fontComp->id});
                            m_engine->getRenderer()->createText(
                                {.fontName = fontComp->id,
                                 .color = {colorComp->r, colorComp->g, colorComp->b, colorComp->a},
                                 .content = textComp->content,
                                 .size = textComp->font_size,
                                 .x = transform->x,
                                 .y = transform->y,
                                 .name = textComp->id});
                        }
                    }
                    else if (type == typeid(ecs::Texture))
                    {
                        const float scale_x = scaleComp ? scaleComp->x : 1.f;
                        const float scale_y = scaleComp ? scaleComp->y : 1.f;

                        m_engine->getRenderer()->createTexture(textureComp->path, textureComp->id);

                        if (transform && textureComp)
                        {
                            if (rectComp)
                            {
                                m_engine->getRenderer()->createSprite(
                                    textureComp->path, transform->x, transform->y, textureComp->id + std::to_string(e),
                                    scale_x, scale_y, static_cast<int>(rectComp->pos_x),
                                    static_cast<int>(rectComp->pos_y), rectComp->size_x, rectComp->size_y);
                            }
                            else
                            {
                                m_engine->getRenderer()->createSprite(textureComp->path, transform->x, transform->y,
                                                                      textureComp->id + std::to_string(e));
                            }
                        }
                    }
                    else if (type == typeid(ecs::Audio))
                    {
                        if (audioComp)
                        {
                            m_engine->getAudio()->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                                              audioComp->id + std::to_string(e));
                        }
                    }
                });
        },
        [](ecs::Registry &reg)
        {
            reg.createEntity()
                .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                .with<ecs::Transform>("transform_title", 10.F, 10.F, 0.F)
                .with<ecs::Color>("color_title", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
                .with<ecs::Text>("id", std::string("RType Client"), 50U)
                .build();
        },
        [](ecs::Registry &reg)
        { reg.createEntity().with<ecs::Audio>("id_audio", Path::Audio::AUDIO_TITLE, 5.F, true, true).build(); },
        [this](ecs::Registry &reg)
        {
            m_fpsEntity = reg.createEntity()
                              .with<ecs::Font>("main_font", Path::Font::FONTS_RTYPE)
                              .with<ecs::Transform>("transform_fps", 10.F, 70.F, 0.F)
                              .with<ecs::Color>("color_fps", WHITE.r, WHITE.g, WHITE.b, WHITE.a)
                              .with<ecs::Text>("id_text", std::string("RType Client"), 20U)
                              .build();
        },
        [this](ecs::Registry &reg)
        {
            m_playerEntity = reg.createEntity()
                                 .with<ecs::Transform>("player_transform", 200.F, 100.F, 0.F)
                                 .with<ecs::Velocity>("player_velocity", 500.F, 500.F)
                                 .with<ecs::Rect>("player_rect", 0.F, 0.F, 33, 20)
                                 .with<ecs::Scale>("player_scale", 2.F, 2.F)
                                 .with<ecs::Texture>("player_texture", Path::Texture::TEXTURE_PLAYER)
                                 .build();
        },
        [](ecs::Registry &reg)
        {
            for (int i = 0; i < 100; i++)
            {
                reg.createEntity()
                    .with<ecs::Pixel>("star_point_" + std::to_string(i))
                    .with<ecs::Transform>("star_point_transform", 0.F, 0.F, 0.F)
                    .with<ecs::Velocity>("star_vel", -20.F - static_cast<float>(std::rand() % 30), 0.F)
                    .with<ecs::Color>("star_color", static_cast<unsigned char>(100U), static_cast<unsigned char>(100U),
                                      static_cast<unsigned char>(200U), static_cast<unsigned char>(255U))
                    .build();
            }
        });
}