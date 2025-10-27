#include "ECS/Component.hpp"
#include "RTypeClientSolo/Utils/HitboxUtils.hpp"
#include "RTypeShared/GameConfig.hpp"
#include "Utils/Common.hpp"

gme::GameSolo::GameSolo(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                        const std::shared_ptr<eng::IAudio> &audio, const int skinIndex, bool &showDebug)
    : AScene(assignedId), m_renderer(renderer), m_audio(audio), m_skinIndex(skinIndex), m_showDebug(showDebug)
{
    auto &registry = AScene::getRegistry();

    registry.onComponentAdded(
        [&renderer, &audio, &registry](const ecs::Entity e, const std::type_info &type)
        {
            const auto *audioComp = registry.getComponent<ecs::Audio>(e);
            const auto *colorComp = registry.getComponent<ecs::Color>(e);
            const auto *fontComp = registry.getComponent<ecs::Font>(e);
            const auto *rectComp = registry.getComponent<ecs::Rect>(e);
            const auto *scaleComp = registry.getComponent<ecs::Scale>(e);
            const auto *textComp = registry.getComponent<ecs::Text>(e);
            const auto *textureComp = registry.getComponent<ecs::Texture>(e);
            const auto *transform = registry.getComponent<ecs::Transform>(e);
            const auto *hitBox = registry.getComponent<ecs::Hitbox>(e);

            if (hitBox && transform)
            {
                renderer->createCircleShape({.name = "hitbox_" + std::to_string(e),
                                             .radius = hitBox->radius,
                                             .color = {.r = 255, .g = 0, .b = 0, .a = 100},
                                             .x = transform->x + hitBox->offsetX,
                                             .y = transform->y + hitBox->offsetY,
                                             .outline_thickness = 1.0f,
                                             .outline_color = {.r = 255, .g = 0, .b = 0, .a = 200}});
            }
            if (type == typeid(ecs::Text))
            {
                if (textComp && transform && fontComp)
                {
                    renderer->createFont(fontComp->id, fontComp->path);
                    renderer->createText(
                        {.font_name = fontComp->id,
                         .color = {.r = colorComp->r, .g = colorComp->g, .b = colorComp->b, .a = colorComp->a},
                         .content = textComp->content,
                         .size = textComp->font_size,
                         .x = transform->x,
                         .y = transform->y,
                         .name = textComp->id});
                }
            }
            else if (type == typeid(ecs::Texture))
            {
                const float scale_x = scaleComp ? scaleComp->x : 1.F;
                const float scale_y = scaleComp ? scaleComp->y : 1.F;

                renderer->createTexture(textureComp->id, textureComp->path);

                if (transform && textureComp)
                {
                    if (rectComp)
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y, scale_x, scale_y, static_cast<int>(rectComp->pos_x),
                                               static_cast<int>(rectComp->pos_y), rectComp->size_x, rectComp->size_y);
                    }
                    else
                    {
                        renderer->createSprite(textureComp->id + std::to_string(e), textureComp->id, transform->x,
                                               transform->y);
                    }
                }
            }
            else if (type == typeid(ecs::Audio))
            {
                if (audioComp)
                {
                    audio->createAudio(audioComp->path, audioComp->volume, audioComp->loop,
                                       audioComp->id + std::to_string(e));
                }
            }
        });

    registry.createEntity().with<ecs::Score>("score", 0).build();

    m_playerEntity = createPlayer(registry);
    m_stageManager = std::make_unique<StageManager>();

    const auto beginSoundEntity = registry.createEntity()
                                      .with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 10.0F, true, true)
                                      .build();
}

void gme::GameSolo::handlePlayerInputs(ecs::Registry &registry, const float dt)
{
    auto *playerTransform = registry.getComponent<ecs::Transform>(m_playerEntity);
    auto *playerVelocity = registry.getComponent<ecs::Velocity>(m_playerEntity);

    if (!playerTransform || !playerVelocity)
        return;

    constexpr float diagonal_speed = GameConfig::Player::SPEED * GameConfig::Player::DIAGONAL_SPEED_MULTIPLIER;

    playerVelocity->x = 0.0f;
    playerVelocity->y = 0.0f;

    const bool up = m_keysPressed[eng::Key::Up];
    const bool down = m_keysPressed[eng::Key::Down];
    const bool left = m_keysPressed[eng::Key::Left];
    const bool right = m_keysPressed[eng::Key::Right];
    if (up && right)
    {
        playerVelocity->x = diagonal_speed;
        playerVelocity->y = -diagonal_speed;
    }
    else if (up && left)
    {
        playerVelocity->x = -diagonal_speed;
        playerVelocity->y = -diagonal_speed;
    }
    else if (down && right)
    {
        playerVelocity->x = diagonal_speed;
        playerVelocity->y = diagonal_speed;
    }
    else if (down && left)
    {
        playerVelocity->x = -diagonal_speed;
        playerVelocity->y = diagonal_speed;
    }
    else
    {
        if (up)
            playerVelocity->y = -GameConfig::Player::SPEED;
        if (down)
            playerVelocity->y = GameConfig::Player::SPEED;
        if (left)
            playerVelocity->x = -GameConfig::Player::SPEED;
        if (right)
            playerVelocity->x = GameConfig::Player::SPEED;
    }

    playerTransform->x += playerVelocity->x * dt;
    playerTransform->y += playerVelocity->y * dt;
    playerTransform->x = std::max(playerTransform->x, 0.F);
    playerTransform->y = std::max(playerTransform->y, 0.F);

    auto [width, height] = m_renderer->getWindowSize();
    const float maxX = static_cast<float>(width) - GameConfig::Player::SPRITE_WIDTH * GameConfig::Player::SCALE;
    const float maxY = static_cast<float>(height) - GameConfig::Player::SPRITE_HEIGHT * GameConfig::Player::SCALE;

    playerTransform->x = std::min(playerTransform->x, maxX);
    playerTransform->y = std::min(playerTransform->y, maxY);
}

void gme::GameSolo::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();
    handlePlayerInputs(reg, dt);
    m_stageManager->update(reg, dt, size);
}

void gme::GameSolo::event(const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Down)
            {
                m_keysPressed[eng::Key::Down] = true;
            }
            if (event.key == eng::Key::Up)
            {
                m_keysPressed[eng::Key::Up] = true;
            }
            if (event.key == eng::Key::Left)
            {
                m_keysPressed[eng::Key::Left] = true;
            }
            if (event.key == eng::Key::Right)
            {
                m_keysPressed[eng::Key::Right] = true;
            }
            break;

        case eng::EventType::KeyReleased:
            if (event.key == eng::Key::Down)
            {
                m_keysPressed[eng::Key::Down] = false;
            }
            if (event.key == eng::Key::Up)
            {
                m_keysPressed[eng::Key::Up] = false;
            }
            if (event.key == eng::Key::Left)
            {
                m_keysPressed[eng::Key::Left] = false;
            }
            if (event.key == eng::Key::Right)
            {
                m_keysPressed[eng::Key::Right] = false;
            }
            break;

        default:
            break;
    }
}

void gme::GameSolo::updatePlayerSkin()
{
    auto &registry = getRegistry();

    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_playerEntity); playerRect != nullptr)
    {
        const float skinPosY = m_skinIndex * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}

ecs::Entity gme::GameSolo::createPlayer(ecs::Registry &registry)
{
    auto [offsetX, offsetY] = Utils::calculateHitboxOffsets(
        GameConfig::Player::SPRITE_WIDTH, GameConfig::Player::SPRITE_HEIGHT, GameConfig::Player::SCALE);

    return registry.createEntity()
        .with<ecs::Transform>("player_transform", 200.0F, 100.0F, 0.F)
        .with<ecs::Velocity>("player_velocity", 0.F, 0.F)
        .with<ecs::Rect>("player_rect", 0.F, 0.F, static_cast<int>(GameConfig::Player::SPRITE_WIDTH),
                         static_cast<int>(GameConfig::Player::SPRITE_HEIGHT))
        .with<ecs::Scale>("player_scale", GameConfig::Player::SCALE, GameConfig::Player::SCALE)
        .with<ecs::Texture>("player_texture", utl::Path::Texture::TEXTURE_PLAYER)
        .with<ecs::Player>("player", true)
        .with<ecs::BeamCharge>("beam_charge", 0.0f, GameConfig::Beam::MAX_CHARGE)
        .with<ecs::Hitbox>("player_hitbox", GameConfig::Hitbox::PLAYER_RADIUS, offsetX, offsetY)
        .build();
}
