#include "RTypeClientMulti/Scenes/GameMulti.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "RTypeShared/GameConfig.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"
#include <algorithm>

#include <ranges>

gme::GameMulti::GameMulti(const eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                          const std::shared_ptr<eng::IAudio> &audio, const float skinIndex, bool &showDebug,
                          const uint32_t sessionId)
    : AScene(assignedId), m_audio(audio), m_renderer(renderer), m_skinIndex(skinIndex), m_showDebug(showDebug),
      m_sessionId(sessionId)
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

    m_playerController = std::make_unique<PlayerControllerMulti>(renderer, m_sessionId);

    m_localPlayerEntity = m_playerController->createPlayer(registry, 200.F, 100.F);

    registry.createEntity().with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 1.0F, false, true).build();

    setupEventSubscriptions();
}

void gme::GameMulti::setupEventSubscriptions() const
{
    utl::EventBus &eventBus = utl::EventBus::getInstance();
    eventBus.registerComponent(m_eventComponentId, "GameMulti");
    eventBus.subscribe(m_eventComponentId, utl::EventType::PLAYER_INPUT_RECEIVED);
    eventBus.subscribe(m_eventComponentId, utl::EventType::WORLD_STATE_RECEIVED);
}

void gme::GameMulti::processEventBus()
{
    auto &eventBus = utl::EventBus::getInstance();
    for (std::vector<utl::Event> events = eventBus.consumeForTarget(m_eventComponentId); const auto &event : events)
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

void gme::GameMulti::handlePlayerInputReceived(const utl::Event &event) {}

void gme::GameMulti::handleWorldStateUpdate(const utl::Event &event)
{
    try
    {
        rnp::Serializer deserializer(event.data);
        rnp::PacketWorldState worldState = deserializer.deserializeWorldState();

        auto &registry = getRegistry();

        for (const auto &entityState : worldState.entities)
        {
            if (entityState.id == m_sessionId)
            {
                if (auto *transform = registry.getComponent<ecs::Transform>(m_localPlayerEntity))
                {
                    float deltaX = std::abs(transform->x - entityState.x);
                    float deltaY = std::abs(transform->y - entityState.y);
                    const float CORRECTION_THRESHOLD = 5.0f;

                    if (deltaX > CORRECTION_THRESHOLD || deltaY > CORRECTION_THRESHOLD)
                    {
                        float t = 0.2f;
                        transform->x = transform->x + t * (entityState.x - transform->x);
                        transform->y = transform->y + t * (entityState.y - transform->y);
                    }
                }
                if (auto *velocity = registry.getComponent<ecs::Velocity>(m_localPlayerEntity))
                {
                    velocity->x = entityState.vx;
                    velocity->y = entityState.vy;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
            {
                if (m_remotePlayers.find(entityState.id) == m_remotePlayers.end())
                {
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

                    m_remotePlayerData[entityState.id] = {.targetX = entityState.x,
                                                          .targetY = entityState.y,
                                                          .targetVx = entityState.vx,
                                                          .targetVy = entityState.vy,
                                                          .currentX = entityState.x,
                                                          .currentY = entityState.y,
                                                          .smoothFactor = REMOTE_PLAYER_SMOOTH_FACTOR,
                                                          .targetRotation = 0.0f,
                                                          .currentRotation = 0.0f};
                }
                else
                {
                    m_remotePlayerData[entityState.id].targetX = entityState.x;
                    m_remotePlayerData[entityState.id].targetY = entityState.y;
                    m_remotePlayerData[entityState.id].targetVx = entityState.vx;
                    m_remotePlayerData[entityState.id].targetVy = entityState.vy;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PROJECTILE))
            {
                if (m_projectileEntities.find(entityState.id) == m_projectileEntities.end())
                {
                    ecs::Entity projectile =
                        registry.createEntity()
                            .with<ecs::Transform>("projectile_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("projectile_velocity_" + std::to_string(entityState.id),
                                                 entityState.vx, entityState.vy)
                            .with<ecs::Rect>("projectile_rect_" + std::to_string(entityState.id), 0.F, 0.F, 20, 10)
                            .with<ecs::Scale>("projectile_scale_" + std::to_string(entityState.id), 1.0f, 1.0f)
                            .with<ecs::Texture>("projectile_texture_" + std::to_string(entityState.id),
                                                utl::Path::Texture::TEXTURE_PLAYER)
                            .build();

                    m_projectileEntities[entityState.id] = projectile;

                    m_projectileData[entityState.id] = {.targetX = entityState.x,
                                                        .targetY = entityState.y,
                                                        .targetVx = entityState.vx,
                                                        .targetVy = entityState.vy,
                                                        .currentX = entityState.x,
                                                        .currentY = entityState.y,
                                                        .smoothFactor = PROJECTILE_SMOOTH_FACTOR,
                                                        .targetRotation = 0.0f,
                                                        .currentRotation = 0.0f};
                }
                else
                {
                    m_projectileData[entityState.id].targetX = entityState.x;
                    m_projectileData[entityState.id].targetY = entityState.y;
                    m_projectileData[entityState.id].targetVx = entityState.vx;
                    m_projectileData[entityState.id].targetVy = entityState.vy;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::ENEMY))
            {
                if (m_enemyEntities.find(entityState.id) == m_enemyEntities.end())
                {
                    ecs::Entity enemy =
                        registry.createEntity()
                            .with<ecs::Transform>("enemy_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("enemy_velocity_" + std::to_string(entityState.id), entityState.vx,
                                                 entityState.vy)
                            .with<ecs::Rect>("enemy_rect_" + std::to_string(entityState.id), 0.F, 0.F, 50, 50)
                            .with<ecs::Scale>("enemy_scale_" + std::to_string(entityState.id), 1.0f, 1.0f)
                            .with<ecs::Texture>("enemy_texture_" + std::to_string(entityState.id),
                                                utl::Path::Texture::TEXTURE_PLAYER)
                            .build();

                    m_enemyEntities[entityState.id] = enemy;

                    m_enemyData[entityState.id] = {.targetX = entityState.x,
                                                   .targetY = entityState.y,
                                                   .targetVx = entityState.vx,
                                                   .targetVy = entityState.vy,
                                                   .currentX = entityState.x,
                                                   .currentY = entityState.y,
                                                   .smoothFactor = ENEMY_SMOOTH_FACTOR,
                                                   .targetRotation = 0.0f,
                                                   .currentRotation = 0.0f};
                }
                else
                {
                    m_enemyData[entityState.id].targetX = entityState.x;
                    m_enemyData[entityState.id].targetY = entityState.y;
                    m_enemyData[entityState.id].targetVx = entityState.vx;
                    m_enemyData[entityState.id].targetVy = entityState.vy;
                }
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

    updateInterpolation(m_remotePlayerData, m_remotePlayers, REMOTE_PLAYER_SMOOTH_FACTOR, dt, reg);
    updateInterpolation(m_projectileData, m_projectileEntities, PROJECTILE_SMOOTH_FACTOR, dt, reg);
    updateInterpolation(m_enemyData, m_enemyEntities, ENEMY_SMOOTH_FACTOR, dt, reg);

    processEventBus();
}

void gme::GameMulti::event(const eng::Event &event)
{
    auto &reg = getRegistry();
    m_playerController->handleInput(reg, event);
}

void gme::GameMulti::updateInterpolation(std::unordered_map<uint32_t, InterpolationData> &dataMap,
                                         std::unordered_map<uint32_t, ecs::Entity> &entityMap, float smoothFactor,
                                         float dt, ecs::Registry &registry)
{
    for (auto &[entityId, interpData] : dataMap)
    {
        if (&dataMap == &m_remotePlayerData && entityId == m_sessionId)
            continue;

        if (entityMap.find(entityId) != entityMap.end())
        {
            ecs::Entity entity = entityMap[entityId];
            if (auto *transform = registry.getComponent<ecs::Transform>(entity))
            {
                interpData.currentX += (interpData.targetX - interpData.currentX) * interpData.smoothFactor;
                interpData.currentY += (interpData.targetY - interpData.currentY) * interpData.smoothFactor;

                transform->x = interpData.currentX;
                transform->y = interpData.currentY;

                if (auto *velocity = registry.getComponent<ecs::Velocity>(entity))
                {
                    velocity->x = interpData.targetVx;
                    velocity->y = interpData.targetVy;
                }

                interpData.currentRotation +=
                    (interpData.targetRotation - interpData.currentRotation) * interpData.smoothFactor;
                transform->rotation = interpData.currentRotation;
            }
        }
    }
}

void gme::GameMulti::updatePlayerSkin()
{
    auto &registry = getRegistry();

    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity); playerRect != nullptr)
    {
        const float skinPosY = m_skinIndex * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}
