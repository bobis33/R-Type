#include "RTypeClientMulti/Scenes/GameMulti.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "RTypeShared/GameConfig.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"

gme::GameMulti::GameMulti(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                          const std::shared_ptr<eng::IAudio> &audio, const float skinIndex, bool &showDebug,
                          const uint32_t lobbyId, const uint32_t sessionId)
    : AScene(assignedId), m_audio(audio), m_renderer(renderer), m_skinIndex(skinIndex), m_showDebug(showDebug),
      m_lobbyId(lobbyId), m_sessionId(sessionId)
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
                float hitboxX = transform->x + hitBox->offsetX;
                float hitboxY = transform->y + hitBox->offsetY;
                renderer->createCircleShape({.name = "hitbox_" + std::to_string(e),
                                             .radius = hitBox->radius,
                                             .color = {.r = 255, .g = 0, .b = 0, .a = 100},
                                             .x = hitboxX,
                                             .y = hitboxY,
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

    m_playerController = std::make_unique<PlayerControllerMulti>(renderer, m_sessionId);

    m_localPlayerEntity = m_playerController->createPlayer(registry, 200.F, 100.F);

    auto beginSoundEntity = registry.createEntity()
                                .with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 1.0F, false, false)
                                .build();
    if (auto *audioComp = registry.getComponent<ecs::Audio>(beginSoundEntity))
    {
        audioComp->play = true;
    }

    setupEventSubscriptions();
}

void gme::GameMulti::setupEventSubscriptions()
{
    utl::EventBus &eventBus = utl::EventBus::getInstance();
    eventBus.registerComponent(m_eventComponentId, "GameMulti");
    eventBus.subscribe(m_eventComponentId, utl::EventType::PLAYER_INPUT_RECEIVED);
    eventBus.subscribe(m_eventComponentId, utl::EventType::WORLD_STATE_RECEIVED);
}

void gme::GameMulti::processEventBus()
{
    auto &eventBus = utl::EventBus::getInstance();
    std::vector<utl::Event> events = eventBus.consumeForTarget(m_eventComponentId);
    for (const auto &event : events)
    {
        switch (event.type)
        {
            case utl::EventType::PLAYER_INPUT_RECEIVED:
                handlePlayerInputReceived(event);
                break;
            case utl::EventType::WORLD_STATE_RECEIVED:
                handleWorldStateUpdate(event);
                break;
            default:
                break;
        }
    }
}

void gme::GameMulti::handlePlayerInputReceived(const utl::Event &event)
{
    // This is used when we receive our own input back from server (for validation)
    // For now, we don't need to do anything special here
}

void gme::GameMulti::handleWorldStateUpdate(const utl::Event &event)
{
    try
    {
        rnp::Serializer deserializer(event.data);
        rnp::PacketWorldState worldState = deserializer.deserializeWorldState();

        auto &registry = getRegistry();

        // Update each entity received from server
        for (const auto &entityState : worldState.entities)
        {
            if (entityState.id == m_sessionId)
            {
                // Local player: Gentle reconciliation to fix position drift
                if (auto *transform = registry.getComponent<ecs::Transform>(m_localPlayerEntity))
                {
                    float deltaX = std::abs(transform->x - entityState.x);
                    float deltaY = std::abs(transform->y - entityState.y);
                    const float CORRECTION_THRESHOLD = 5.0f; // Only correct if difference is significant

                    if (deltaX > CORRECTION_THRESHOLD || deltaY > CORRECTION_THRESHOLD)
                    {
                        // Gentle correction (20% blend per frame)
                        float t = 0.2f;
                        transform->x = transform->x + t * (entityState.x - transform->x);
                        transform->y = transform->y + t * (entityState.y - transform->y);
                    }
                }
                if (auto *velocity = registry.getComponent<ecs::Velocity>(m_localPlayerEntity))
                {
                    // Sync velocity from server
                    velocity->x = entityState.vx;
                    velocity->y = entityState.vy;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
            {
                // Update or create remote player
                if (m_remotePlayers.find(entityState.id) == m_remotePlayers.end())
                {
                    // Create new remote player entity
                    ecs::Entity remotePlayer =
                        registry.createEntity()
                            .with<ecs::Transform>("remote_player_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("remote_velocity_" + std::to_string(entityState.id), entityState.vx,
                                                 entityState.vy)
                            .with<ecs::Rect>("remote_rect_" + std::to_string(entityState.id), 0.F, 0.F,
                                             static_cast<int>(GameConfig::Player::SPRITE_WIDTH),
                                             static_cast<int>(GameConfig::Player::SPRITE_HEIGHT))
                            .with<ecs::Scale>("remote_scale_" + std::to_string(entityState.id),
                                              GameConfig::Player::SCALE, GameConfig::Player::SCALE)
                            .with<ecs::Texture>("remote_texture_" + std::to_string(entityState.id),
                                                utl::Path::Texture::TEXTURE_PLAYER)
                            .with<ecs::Player>("remote_player_comp_" + std::to_string(entityState.id), false)
                            .build();
                    m_remotePlayers[entityState.id] = remotePlayer;
                }
                else
                {
                    // Remote player: Apply directly with slight smoothing
                    ecs::Entity remotePlayer = m_remotePlayers[entityState.id];
                    if (auto *transform = registry.getComponent<ecs::Transform>(remotePlayer))
                    {
                        // Store previous position for interpolation
                        float prevX = transform->x;
                        float prevY = transform->y;

                        // Store interpolation data
                        if (m_interpolationData.find(entityState.id) == m_interpolationData.end() ||
                            m_interpolationData[entityState.id].interpolationTime >=
                                m_interpolationData[entityState.id].interpolationDuration)
                        {
                            m_interpolationData[entityState.id] = {
                                .prevX = prevX,
                                .prevY = prevY,
                                .targetX = entityState.x,
                                .targetY = entityState.y,
                                .interpolationTime = 0.0f,
                                .interpolationDuration = 0.05f // 50ms smooth interpolation
                            };
                        }
                    }
                    if (auto *velocity = registry.getComponent<ecs::Velocity>(remotePlayer))
                    {
                        velocity->x = entityState.vx;
                        velocity->y = entityState.vy;
                    }
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PROJECTILE))
            {
                // Handle projectile entities (TODO: implement)
            }
        }
    }
    catch (const std::exception &e)
    {
        utl::Logger::log("GameMulti: Error handling world state: " + std::string(e.what()), utl::LogLevel::WARNING);
    }
}

void gme::GameMulti::update(const float dt, const eng::WindowSize &size)
{
    auto &reg = getRegistry();

    // Client-side prediction: Update local player immediately
    if (m_playerController)
    {
        m_playerController->update(reg, dt);
    }

    const auto &audios = reg.getAll<ecs::Audio>();

    for (const auto &audio : audios)
    {
        if (!audio.second.play && (m_audio->isPlaying(audio.second.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.second.id);
        }
    }

    // Update interpolation for remote players ONLY (not local player)
    for (auto &[playerId, interpData] : m_interpolationData)
    {
        // Skip local player
        if (playerId == m_sessionId)
            continue;

        if (m_remotePlayers.find(playerId) != m_remotePlayers.end())
        {
            ecs::Entity remotePlayer = m_remotePlayers[playerId];
            if (auto *transform = reg.getComponent<ecs::Transform>(remotePlayer))
            {
                interpData.interpolationTime += dt;
                float t = std::min(interpData.interpolationTime / interpData.interpolationDuration, 1.0f);

                // Linear interpolation
                transform->x = interpData.prevX + (interpData.targetX - interpData.prevX) * t;
                transform->y = interpData.prevY + (interpData.targetY - interpData.prevY) * t;
            }
        }
    }

    processEventBus();
}

void gme::GameMulti::event(const eng::Event &event)
{
    auto &reg = getRegistry();
    m_playerController->handleInput(reg, event);
}

void gme::GameMulti::updatePlayerSkin()
{
    auto &registry = getRegistry();
    auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity);

    if (playerRect != nullptr)
    {
        const float skinPosY = static_cast<float>(m_skinIndex) * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}
