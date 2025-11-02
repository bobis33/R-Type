#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "RTypeClientMulti/Utils/HitboxUtils.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

ecs::Entity gme::PlayerControllerMulti::createPlayer(ecs::Registry &registry, float x, float y)
{
    auto [offsetX, offsetY] = Utils::calculateHitboxOffsets(
        utl::GameConfig::Player::SPRITE_WIDTH, utl::GameConfig::Player::SPRITE_HEIGHT, utl::GameConfig::Player::SCALE);

    m_playerEntity =
        registry.createEntity()
            .with<ecs::Transform>("player_transform", x, y, 0.F)
            .with<ecs::Velocity>("player_velocity", 0.F, 0.F)
            .with<ecs::Rect>("player_rect", 0.F, 0.F, static_cast<int>(utl::GameConfig::Player::SPRITE_WIDTH),
                             static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT))
            .with<ecs::Scale>("player_scale", utl::GameConfig::Player::SCALE, utl::GameConfig::Player::SCALE)
            .with<ecs::Texture>("player_texture", utl::Path::Texture::TEXTURE_PLAYER)
            .with<ecs::Player>("player", true)
            .with<ecs::Health>("player_health", 100.0f, 100.0f)
            .with<ecs::BeamCharge>("beam_charge", 0.0f, utl::GameConfig::Beam::MAX_CHARGE)
            .with<ecs::Hitbox>("player_hitbox", utl::GameConfig::Hitbox::PLAYER_RADIUS, offsetX, offsetY)
            .build();
    return m_playerEntity;
}

void gme::PlayerControllerMulti::handleInput(ecs::Registry &registry, const eng::Event &event)
{
    switch (event.type)
    {
        case eng::EventType::KeyPressed:
            if (event.key == eng::Key::Up)
            {
                m_keysPressed[eng::Key::Up] = true;
            }
            if (event.key == eng::Key::Down)
            {
                m_keysPressed[eng::Key::Down] = true;
            }
            if (event.key == eng::Key::Left)
            {
                m_keysPressed[eng::Key::Left] = true;
            }
            if (event.key == eng::Key::Right)
            {
                m_keysPressed[eng::Key::Right] = true;
            }
            if (event.key == eng::Key::Space)
            {
                m_keysPressed[eng::Key::Space] = true;
            }
            break;

        case eng::EventType::KeyReleased:
            if (event.key == eng::Key::Up)
            {
                m_keysPressed[eng::Key::Up] = false;
            }
            if (event.key == eng::Key::Down)
            {
                m_keysPressed[eng::Key::Down] = false;
            }
            if (event.key == eng::Key::Left)
            {
                m_keysPressed[eng::Key::Left] = false;
            }
            if (event.key == eng::Key::Right)
            {
                m_keysPressed[eng::Key::Right] = false;
            }
            if (event.key == eng::Key::Space)
            {
                m_keysPressed[eng::Key::Space] = false;
            }
            break;

        default:
            break;
    }

    sendInputsIfChanged();
}

void gme::PlayerControllerMulti::sendInputsIfChanged()
{
    // Send inputs immediately for better responsiveness
    // No throttling for shoot input

    auto checkKey = [this](const eng::Key key) -> bool
    {
        auto it = m_keysPressed.find(key);
        return it != m_keysPressed.end() && it->second;
    };

    const bool up = checkKey(eng::Key::Up);
    const bool down = checkKey(eng::Key::Down);
    const bool left = checkKey(eng::Key::Left);
    const bool right = checkKey(eng::Key::Right);
    const bool shoot = checkKey(eng::Key::Space);

    sendInputToServer(up, down, left, right, shoot);
}

void gme::PlayerControllerMulti::sendInputToServer(bool up, bool down, bool left, bool right, bool shoot) const
{
    std::vector<std::uint8_t> inputData;
    inputData.push_back(up ? 1 : 0);
    inputData.push_back(down ? 1 : 0);
    inputData.push_back(left ? 1 : 0);
    inputData.push_back(right ? 1 : 0);
    inputData.push_back(shoot ? 1 : 0);
    // Create EventRecord
    rnp::EventRecord eventRecord;
    eventRecord.entityId = 0;
    eventRecord.type = rnp::EventType::INPUT;
    eventRecord.data = inputData;

    // Serialize and send
    std::vector<rnp::EventRecord> events;
    events.push_back(eventRecord);

    rnp::Serializer serializer;
    serializer.serializeEntityEvents(events);

    // Publish to event bus to send to server
    m_eventBus.publish(utl::EventType::SEND_ENTITY_EVENT, serializer.getData(), m_componentId, utl::NETWORK_CLIENT);
}

void gme::PlayerControllerMulti::update(ecs::Registry &registry, float dt)
{
    const auto keyboardEntities = registry.getAll<ecs::KeyboardInput>();
    ecs::Entity keyboardEntity = ecs::INVALID_ENTITY;
    if (!keyboardEntities.empty())
    {
        keyboardEntity = keyboardEntities.begin()->first;
    }
    else
    {
        keyboardEntity = registry.createEntity().with<ecs::KeyboardInput>("keyboard_input").build();
    }

    if (auto *keyboardInput = registry.getComponent<ecs::KeyboardInput>(keyboardEntity))
    {
        auto checkKey = [this](const eng::Key key) -> bool
        {
            auto it = m_keysPressed.find(key);
            return it != m_keysPressed.end() && it->second;
        };

        keyboardInput->space_pressed = checkKey(eng::Key::Space);
        keyboardInput->up_pressed = checkKey(eng::Key::Up);
        keyboardInput->down_pressed = checkKey(eng::Key::Down);
        keyboardInput->left_pressed = checkKey(eng::Key::Left);
        keyboardInput->right_pressed = checkKey(eng::Key::Right);
    }

    // Client-side prediction: Apply inputs immediately to local player
    if (auto *transform = registry.getComponent<ecs::Transform>(m_playerEntity))
    {
        if (auto *velocity = registry.getComponent<ecs::Velocity>(m_playerEntity))
        {
            // Calculate velocity based on current key states
            constexpr float SPEED = 500.0f;
            velocity->x = 0.0f;
            velocity->y = 0.0f;

            auto checkKey = [this](const eng::Key key) -> bool
            {
                auto it = m_keysPressed.find(key);
                return it != m_keysPressed.end() && it->second;
            };

            const bool up = checkKey(eng::Key::Up);
            const bool down = checkKey(eng::Key::Down);
            const bool left = checkKey(eng::Key::Left);
            const bool right = checkKey(eng::Key::Right);

            if (up)
            {
                velocity->y = -SPEED;
            }
            if (down)
            {
                velocity->y = SPEED;
            }
            if (left)
            {
                velocity->x = -SPEED;
            }
            if (right)
            {
                velocity->x = SPEED;
            }

            if (velocity->x != 0.0f && velocity->y != 0.0f)
            {
                velocity->x *= 0.707f;
                velocity->y *= 0.707f;
            }

            transform->x += velocity->x * dt;
            transform->y += velocity->y * dt;

            transform->x = std::max(0.0f, std::min(1920.0f, transform->x));
            transform->y = std::max(0.0f, std::min(1080.0f, transform->y));
        }
    }
}

bool gme::PlayerControllerMulti::isSpacePressed() const
{
    auto it = m_keysPressed.find(eng::Key::Space);
    return it != m_keysPressed.end() && it->second;
}
