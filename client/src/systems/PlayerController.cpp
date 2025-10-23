#include "Client/Systems/PlayerController.hpp"
#include "Client/Common.hpp"

ecs::Entity cli::PlayerController::createPlayer(ecs::Registry &registry, float x, float y)
{
    m_playerEntity = registry.createEntity()
                         .with<ecs::Transform>("player_transform", x, y, 0.F)
                         .with<ecs::Velocity>("player_velocity", 0.F, 0.F)
                         .with<ecs::Rect>("player_rect", 0.F, 0.F, static_cast<int>(GameConfig::Player::SPRITE_WIDTH),
                                          static_cast<int>(GameConfig::Player::SPRITE_HEIGHT))
                         .with<ecs::Scale>("player_scale", GameConfig::Player::SCALE, GameConfig::Player::SCALE)
                         .with<ecs::Texture>("player_texture", Path::Texture::TEXTURE_PLAYER)
                         .with<ecs::Player>("player", true)
                         .with<ecs::BeamCharge>("beam_charge", 0.0f, GameConfig::Beam::MAX_CHARGE)
                         .with<ecs::Hitbox>("player_hitbox", GameConfig::Hitbox::PLAYER_RADIUS)
                         .build();
    return m_playerEntity;
}

void cli::PlayerController::handleInput(ecs::Registry &registry, const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
                m_keysPressed[eng::Key::Up] = true;
            if (event.key == eng::Key::Down)
                m_keysPressed[eng::Key::Down] = true;
            if (event.key == eng::Key::Left)
                m_keysPressed[eng::Key::Left] = true;
            if (event.key == eng::Key::Right)
                m_keysPressed[eng::Key::Right] = true;
            if (event.key == eng::Key::Space)
                m_keysPressed[eng::Key::Space] = true;
            break;

        case eng::EventType::KeyReleased:
            if (event.key == eng::Key::Up)
                m_keysPressed[eng::Key::Up] = false;
            if (event.key == eng::Key::Down)
                m_keysPressed[eng::Key::Down] = false;
            if (event.key == eng::Key::Left)
                m_keysPressed[eng::Key::Left] = false;
            if (event.key == eng::Key::Right)
                m_keysPressed[eng::Key::Right] = false;
            if (event.key == eng::Key::Space)
                m_keysPressed[eng::Key::Space] = false;
            break;

        default:
            break;
    }
}

void cli::PlayerController::update(ecs::Registry &registry, float dt)
{
    auto *playerTransform = registry.getComponent<ecs::Transform>(m_playerEntity);
    auto *playerVelocity = registry.getComponent<ecs::Velocity>(m_playerEntity);

    if (!playerTransform || !playerVelocity)
        return;

    float speed = GameConfig::Player::SPEED;
    float diagonal_speed = speed * GameConfig::Player::DIAGONAL_SPEED_MULTIPLIER;

    playerVelocity->x = 0.0f;
    playerVelocity->y = 0.0f;

    bool up = m_keysPressed[eng::Key::Up];
    bool down = m_keysPressed[eng::Key::Down];
    bool left = m_keysPressed[eng::Key::Left];
    bool right = m_keysPressed[eng::Key::Right];
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
            playerVelocity->y = -speed;
        if (down)
            playerVelocity->y = speed;
        if (left)
            playerVelocity->x = -speed;
        if (right)
            playerVelocity->x = speed;
    }

    playerTransform->x += playerVelocity->x * dt;
    playerTransform->y += playerVelocity->y * dt;
    playerTransform->x = std::max(playerTransform->x, 0.F);
    playerTransform->y = std::max(playerTransform->y, 0.F);

    auto windowSize = m_renderer->getWindowSize();
    float maxX = static_cast<float>(windowSize.width) - GameConfig::Player::SPRITE_WIDTH * GameConfig::Player::SCALE;
    float maxY = static_cast<float>(windowSize.height) - GameConfig::Player::SPRITE_HEIGHT * GameConfig::Player::SCALE;

    playerTransform->x = std::min(playerTransform->x, maxX);
    playerTransform->y = std::min(playerTransform->y, maxY);
}
