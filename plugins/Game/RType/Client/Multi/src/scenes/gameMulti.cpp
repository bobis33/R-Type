#include <algorithm>
#include <ranges>
#include <set>

#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Managers/StageManager.hpp"
#include "RTypeClientMulti/Scenes/GameMulti.hpp"
#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

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
    
    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity))
    {
        uint32_t skinIndex = 0;
        if (m_playerSkinMap.find(m_sessionId) != m_playerSkinMap.end())
        {
            skinIndex = m_playerSkinMap[m_sessionId];
        }
        float skinPosY = static_cast<float>(skinIndex) * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }

    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity))
    {
        uint32_t skinIndex = 0;
        if (m_playerSkinMap.contains(m_sessionId))
        {
            skinIndex = m_playerSkinMap[m_sessionId];
        }
        const float skinPosY = static_cast<float>(skinIndex) * utl::GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }

    const auto beginSoundEntity = registry.createEntity()
                                      .with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 1.0F, false, false)
                                      .build();
    if (auto *audioComp = registry.getComponent<ecs::Audio>(beginSoundEntity))
    {
        audioComp->play = true;
    }
    m_beginSoundEntity = beginSoundEntity;

    m_stageManager = std::make_unique<StageManager>();

    setupEventSubscriptions();
}

void gme::GameMulti::setupEventSubscriptions() const
{
    utl::EventBus &eventBus = utl::EventBus::getInstance();
    eventBus.registerComponent(m_eventComponentId, "GameMulti");
    eventBus.subscribe(m_eventComponentId, utl::EventType::GAME_START);
    eventBus.subscribe(m_eventComponentId, utl::EventType::PLAYER_INPUT_RECEIVED);
    eventBus.subscribe(m_eventComponentId, utl::EventType::WORLD_STATE_RECEIVED);
}

void gme::GameMulti::processEventBus()
{
    auto &eventBus = utl::EventBus::getInstance();
    for (const std::vector<utl::Event> events = eventBus.consumeForTarget(m_eventComponentId);
         const auto &event : events)
    {
        switch (event.type)
        {
            case utl::EventType::GAME_START:
            {
                break;
            }
            case utl::EventType::PLAYER_INPUT_RECEIVED:
                break;
            case utl::EventType::WORLD_STATE_RECEIVED:
                handleWorldStateUpdate(event);
                break;
            default:
                break;
        }
    }
}

void gme::GameMulti::handleWorldStateUpdate(const utl::Event &event)
{
    try
    {
        rnp::Serializer deserializer(event.data);
        rnp::PacketWorldState worldState = deserializer.deserializeWorldState();

        auto &registry = getRegistry();

        static bool firstWorldState = true;
        if (firstWorldState)
        {
            uint32_t playerIndex = 0;
            std::vector<uint32_t> playerIds;

            playerIds.push_back(m_sessionId);
            for (const auto &entityState : worldState.entities)
            {
                if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
                {
                    if (entityState.id != m_sessionId)
                    {
                        playerIds.push_back(entityState.id);
                    }
                }
            }

            for (uint32_t playerId : playerIds)
            {
                m_playerSkinMap[playerId] = playerIndex;
                playerIndex++;
            }

            firstWorldState = false;
        }

        for (const auto &[id, type, x, y, vx, vy, stateFlags] : worldState.entities)
        {
            if (id == m_sessionId)
            {
                if (auto *transform = registry.getComponent<ecs::Transform>(m_localPlayerEntity))
                {
                    const float deltaX = std::abs(transform->x - x);
                    const float deltaY = std::abs(transform->y - y);

                    if (constexpr float CORRECTION_THRESHOLD = 5.0f;
                        deltaX > CORRECTION_THRESHOLD || deltaY > CORRECTION_THRESHOLD)
                    {
                        float t = 0.2f;
                        transform->x = transform->x + t * (x - transform->x);
                        transform->y = transform->y + t * (y - transform->y);
                    }
                }
                if (auto *velocity = registry.getComponent<ecs::Velocity>(m_localPlayerEntity))
                {
                    velocity->x = vx;
                    velocity->y = vy;
                }
                if (auto *beamCharge = registry.getComponent<ecs::BeamCharge>(m_localPlayerEntity))
                {
                    beamCharge->current_charge = static_cast<float>(stateFlags) / 255.0f;
                }
            }
            else if (type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
            {
                if (const auto remoteIt = m_remotePlayers.find(id); remoteIt == m_remotePlayers.end())
                {
                    uint32_t skinIndex = 0;
                    if (const auto skinIt = m_playerSkinMap.find(id); skinIt != m_playerSkinMap.end())
                    {
                        skinIndex = skinIt->second;
                    }
                    float skinPosY = static_cast<float>(skinIndex) * utl::GameConfig::Player::SPRITE_HEIGHT;

                    const ecs::Entity remotePlayer =
                        registry.createEntity()
                            .with<ecs::Transform>("remote_player_" + std::to_string(id), x, y, 0.F)
                            .with<ecs::Velocity>("remote_velocity_" + std::to_string(id), vx, vy)
                            .with<ecs::Rect>("remote_rect_" + std::to_string(id), 0.F, skinPosY,
                                             static_cast<int>(utl::GameConfig::Player::SPRITE_WIDTH),
                                             static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT))
                            .with<ecs::Scale>("remote_scale_" + std::to_string(id), utl::GameConfig::Player::SCALE,
                                              utl::GameConfig::Player::SCALE)
                            .with<ecs::Texture>("remote_texture_" + std::to_string(id),
                                                utl::Path::Texture::TEXTURE_PLAYER)
                            .with<ecs::Player>("remote_player_comp_" + std::to_string(id), false)
                            .build();
                    m_remotePlayers[id] = remotePlayer;

                    m_remotePlayerData[id] = {.targetX = x,
                                              .targetY = y,
                                              .targetVx = vx,
                                              .targetVy = vy,
                                              .currentX = x,
                                              .currentY = y,
                                              .smoothFactor = REMOTE_PLAYER_SMOOTH_FACTOR,
                                              .targetRotation = 0.0f,
                                              .currentRotation = 0.0f};
                }
                else
                {
                    m_remotePlayerData[id].targetX = x;
                    m_remotePlayerData[id].targetY = y;
                    m_remotePlayerData[id].targetVx = vx;
                    m_remotePlayerData[id].targetVy = vy;
                }
            }
            else if (type == static_cast<std::uint16_t>(rnp::EntityType::PROJECTILE))
            {
                if (const auto projectileIt = m_projectileEntities.find(id); projectileIt == m_projectileEntities.end())
                {
                    const bool isSupercharged = (vx > 1000.0f || std::abs(vx) > 1000.0f);
                    std::string texturePath =
                        isSupercharged ? utl::Path::Texture::TEXTURE_SHOOT_CHARGED : utl::Path::Texture::TEXTURE_SHOOT;

                    auto entityBuilder =
                        registry.createEntity()
                            .with<ecs::Transform>("projectile_" + std::to_string(id), x, y, 0.F)
                            .with<ecs::Velocity>("projectile_velocity_" + std::to_string(id), vx, vy)
                            .with<ecs::Rect>("projectile_rect_" + std::to_string(id), 0.F, 0.F,
                                             isSupercharged ? 29 : 20, isSupercharged ? 24 : 10)
                            .with<ecs::Scale>("projectile_scale_" + std::to_string(id), isSupercharged ? 1.5f : 1.0f,
                                              isSupercharged ? 1.5f : 1.0f)
                            .with<ecs::Texture>("projectile_texture_" + std::to_string(id), texturePath);

                    if (isSupercharged)
                    {
                        entityBuilder.with<ecs::Animation>("projectile_animation_" + std::to_string(id), 0, 4, 0.15f,
                                                           0.0f, 29, 24, 4);
                    }

                    const ecs::Entity projectile = entityBuilder.build();
                    m_projectileEntities[id] = projectile;

                    const auto shootSound =
                        registry.createEntity()
                            .with<ecs::Audio>("projectile_shoot_" + std::to_string(id),
                                              utl::Path::Audio::AUDIO_SUPERCHARGED_SHOT, 1.5F, false, false)
                            .build();
                    if (auto *audioComp = registry.getComponent<ecs::Audio>(shootSound))
                    {
                        audioComp->play = true;
                    }
                }
                else
                {
                    const ecs::Entity projectileEntity = projectileIt->second;
                    auto *transform = registry.getComponent<ecs::Transform>(projectileEntity);
                    auto *velocity = registry.getComponent<ecs::Velocity>(projectileEntity);

                    if (transform != nullptr)
                    {
                        transform->x = x;
                        transform->y = y;

                        if (transform->x > 2000.0f || transform->x < -100.0f)
                        {
                            registry.removeComponent<ecs::Transform>(projectileEntity);
                            registry.removeComponent<ecs::Velocity>(projectileEntity);
                            registry.removeComponent<ecs::Rect>(projectileEntity);
                            registry.removeComponent<ecs::Scale>(projectileEntity);
                            registry.removeComponent<ecs::Texture>(projectileEntity);
                            registry.removeComponent<ecs::Animation>(projectileEntity);
                            m_projectileEntities.erase(projectileIt);
                        }
                        else if (velocity != nullptr)
                        {
                            velocity->x = vx;
                            velocity->y = vy;
                        }
                    }
                    else if (velocity != nullptr)
                    {
                        velocity->x = vx;
                        velocity->y = vy;
                    }
                }
            }
            else if (type == static_cast<std::uint16_t>(rnp::EntityType::ENEMY))
            {
                if (auto enemyIt = m_enemyEntities.find(id); enemyIt == m_enemyEntities.end())
                {
                    const ecs::Entity enemy = registry.createEntity()
                                                  .with<ecs::Transform>("enemy_" + std::to_string(id), x, y, 0.F)
                                                  .with<ecs::Velocity>("enemy_velocity_" + std::to_string(id), vx, vy)
                                                  .with<ecs::Rect>("enemy_rect_" + std::to_string(id), 0.F, 0.F, 50, 50)
                                                  .with<ecs::Scale>("enemy_scale_" + std::to_string(id), 1.0f, 1.0f)
                                                  .with<ecs::Texture>("enemy_texture_" + std::to_string(id),
                                                                      utl::Path::Texture::TEXTURE_PLAYER)
                                                  .build();

                    m_enemyEntities[id] = enemy;

                    m_enemyData[id] = {.targetX = x,
                                       .targetY = y,
                                       .targetVx = vx,
                                       .targetVy = vy,
                                       .currentX = x,
                                       .currentY = y,
                                       .smoothFactor = ENEMY_SMOOTH_FACTOR,
                                       .targetRotation = 0.0f,
                                       .currentRotation = 0.0f};
                }
                else
                {
                    m_enemyData[id].targetX = x;
                    m_enemyData[id].targetY = y;
                    m_enemyData[id].targetVx = vx;
                    m_enemyData[id].targetVy = vy;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::ENEMY))
            {
                if (m_enemyEntities.find(entityState.id) == m_enemyEntities.end())
                {
                    ecs::Entity enemy = registry.createEntity()
                        .with<ecs::Transform>("enemy_" + std::to_string(entityState.id), entityState.x, entityState.y, 0.F)
                        .with<ecs::Velocity>("enemy_velocity_" + std::to_string(entityState.id), entityState.vx, entityState.vy)
                        .with<ecs::Rect>("enemy_rect_" + std::to_string(entityState.id), 0.F, 0.F, 50, 50)
                        .with<ecs::Scale>("enemy_scale_" + std::to_string(entityState.id), 1.0f, 1.0f)
                        .with<ecs::Texture>("enemy_texture_" + std::to_string(entityState.id), utl::Path::Texture::TEXTURE_PLAYER)
                        .build();
                    
                    m_enemyEntities[entityState.id] = enemy;
                    
                    m_enemyData[entityState.id] = {
                        .targetX = entityState.x,
                        .targetY = entityState.y,
                        .targetVx = entityState.vx,
                        .targetVy = entityState.vy,
                        .currentX = entityState.x,
                        .currentY = entityState.y,
                        .smoothFactor = ENEMY_SMOOTH_FACTOR,
                        .targetRotation = 0.0f,
                        .currentRotation = 0.0f
                    };
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
    for (const auto &audios = reg.getAll<ecs::Audio>(); const auto &audio : audios | std::views::values)
    {
        if (!audio.play && audio.loop && (m_audio->isPlaying(audio.id) == eng::Status::Playing))
        {
            m_audio->stopAudio(audio.id);
        }
    }
    if (m_beginSoundEntity != ecs::Entity{} && m_stageManager && !m_bossMusicStarted)
    {
        thread_local std::string beginNameCache;
        beginNameCache.clear();
        beginNameCache = "game_begin";
        beginNameCache += std::to_string(m_beginSoundEntity);

        if (m_audio->isPlaying(beginNameCache) != eng::Status::Playing)
        {
            if (auto *beginAudio = reg.getComponent<ecs::Audio>(m_beginSoundEntity))
            {
                beginAudio->play = false;
            }
            m_stageManager->stopScrolling(reg);
            m_bossMusicEntity = reg.createEntity()
                                    .with<ecs::Audio>("boss_music", utl::Path::Audio::AUDIO_BOSS, 1.0F, true, false)
                                    .build();
            if (auto *bossAudio = reg.getComponent<ecs::Audio>(m_bossMusicEntity))
            {
                bossAudio->loop = true;
                bossAudio->play = true;
            }
            m_bossMusicStarted = true;
            m_bossMusicTimer = 0.0f;
            m_beginSoundEntity = {};
        }
    }
    if (m_bossMusicStarted)
    {
        m_bossMusicTimer += dt;
        if (m_bossMusicTimer >= BOSS_MUSIC_DURATION)
        {
            if (auto *bossAudio = reg.getComponent<ecs::Audio>(m_bossMusicEntity))
            {
                bossAudio->loop = false;
                bossAudio->play = false;
            }
            m_bossMusicStarted = false;
        }
        else
        {
            if (auto *bossAudio = reg.getComponent<ecs::Audio>(m_bossMusicEntity))
            {
                bossAudio->loop = true;
                bossAudio->play = true;
            }
        }
    }

    updateInterpolation(m_remotePlayerData, m_remotePlayers, REMOTE_PLAYER_SMOOTH_FACTOR, dt, reg);
    updateInterpolation(m_projectileData, m_projectileEntities, PROJECTILE_SMOOTH_FACTOR, dt, reg);
    updateInterpolation(m_enemyData, m_enemyEntities, ENEMY_SMOOTH_FACTOR, dt, reg);

    m_stageManager->update(reg, dt, size);

    processEventBus();
}

void gme::GameMulti::event(const eng::Event &event)
{
    auto &reg = getRegistry();
    m_playerController->handleInput(reg, event);
}

void gme::GameMulti::updateInterpolation(std::unordered_map<uint32_t, InterpolationData> &dataMap,
                                         std::unordered_map<uint32_t, ecs::Entity> &entityMap, float smoothFactor,
                                         float dt, ecs::Registry &registry) const
{
    for (auto &[entityId, interpData] : dataMap)
    {
        if (&dataMap == &m_remotePlayerData && entityId == m_sessionId)
        {
            continue;
        }

        if (auto entityIt = entityMap.find(entityId); entityIt != entityMap.end())
        {
            const ecs::Entity entity = entityIt->second;
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
        const float skinPosY = m_skinIndex * utl::GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}
