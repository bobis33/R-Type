#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/Protocol/Protocol.hpp"
#include "Interfaces/Protocol/Serializer.hpp"
#include "RTypeShared/GameConfig.hpp"

ecs::Entity gme::PlayerControllerMulti::createPlayer(ecs::Registry &registry, float x, float y)
{
    m_playerEntity = registry.createEntity()
                         .with<ecs::Transform>("player_transform", x, y, 0.F)
                         .with<ecs::Velocity>("player_velocity", 0.F, 0.F)
                         .with<ecs::Rect>("player_rect", 0.F, 0.F, static_cast<int>(GameConfig::Player::SPRITE_WIDTH),
                                          static_cast<int>(GameConfig::Player::SPRITE_HEIGHT))
                         .with<ecs::Scale>("player_scale", GameConfig::Player::SCALE, GameConfig::Player::SCALE)
                         .with<ecs::Texture>("player_texture", utl::Path::Texture::TEXTURE_PLAYER)
                         .with<ecs::Player>("player", true)
                         .with<ecs::BeamCharge>("beam_charge", 0.0f, GameConfig::Beam::MAX_CHARGE)
                         .build();
    return m_playerEntity;
}

void gme::PlayerControllerMulti::handleInput(ecs::Registry &registry, const eng::Event &event)
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

    sendInputsIfChanged();
}

void gme::PlayerControllerMulti::sendInputsIfChanged()
{
    static float accumulatedTime = 0.0f;
    accumulatedTime += 1.0f / 60.0f;

    if (accumulatedTime < INPUT_THROTTLE_INTERVAL)
    {
        return;
    }

    accumulatedTime = 0.0f;

    auto checkKey = [this](eng::Key key) -> bool
    {
        auto it = m_keysPressed.find(key);
        return it != m_keysPressed.end() && it->second;
    };

    bool up = checkKey(eng::Key::Up);
    bool down = checkKey(eng::Key::Down);
    bool left = checkKey(eng::Key::Left);
    bool right = checkKey(eng::Key::Right);
    bool shoot = checkKey(eng::Key::Space);

    sendInputToServer(up, down, left, right, shoot);
}

void gme::PlayerControllerMulti::sendInputToServer(bool up, bool down, bool left, bool right, bool shoot)
{
    // Create input data
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
    // Client-side prediction: Apply inputs immediately to local player
    if (auto *transform = registry.getComponent<ecs::Transform>(m_playerEntity))
    {
        if (auto *velocity = registry.getComponent<ecs::Velocity>(m_playerEntity))
        {
            // Calculate velocity based on current key states
            const float SPEED = 500.0f;
            velocity->x = 0.0f;
            velocity->y = 0.0f;

            auto checkKey = [this](eng::Key key) -> bool
            {
                auto it = m_keysPressed.find(key);
                return it != m_keysPressed.end() && it->second;
            };

            bool up = checkKey(eng::Key::Up);
            bool down = checkKey(eng::Key::Down);
            bool left = checkKey(eng::Key::Left);
            bool right = checkKey(eng::Key::Right);

            if (up)
                velocity->y = -SPEED;
            if (down)
                velocity->y = SPEED;
            if (left)
                velocity->x = -SPEED;
            if (right)
                velocity->x = SPEED;

            // Normalize diagonal movement
            if (velocity->x != 0.0f && velocity->y != 0.0f)
            {
                velocity->x *= 0.707f;
                velocity->y *= 0.707f;
            }

            // Apply velocity
            transform->x += velocity->x * dt;
            transform->y += velocity->y * dt;

            // Clamp to screen bounds
            transform->x = std::max(0.0f, std::min(1920.0f, transform->x));
            transform->y = std::max(0.0f, std::min(1080.0f, transform->y));
        }
    }
}
