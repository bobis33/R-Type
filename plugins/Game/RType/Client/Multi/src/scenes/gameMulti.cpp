#include <algorithm>
#include <ranges>
#include <set>
#include <vector>

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
        [this, &renderer, &audio, &registry](const ecs::Entity e, const std::type_info &type)
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
                    // Only create font if not already loaded (cache)
                    if (this->m_loadedFonts.find(fontComp->id) == this->m_loadedFonts.end())
                    {
                        renderer->createFont(fontComp->id, fontComp->path);
                        this->m_loadedFonts.insert(fontComp->id);
                    }
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

                // Only create texture if not already loaded (cache)
                if (this->m_loadedTextures.find(textureComp->id) == this->m_loadedTextures.end())
                {
                    renderer->createTexture(textureComp->id, textureComp->path);
                    this->m_loadedTextures.insert(textureComp->id);
                }

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
    m_hudSystem = std::make_unique<HUDSystem>(renderer, registry);

    m_localPlayerEntity = m_playerController->createPlayer(registry, 200.F, 100.F);

    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity))
    {
        uint32_t skinIndex = 0;
        if (m_playerSkinMap.find(m_sessionId) != m_playerSkinMap.end())
        {
            skinIndex = m_playerSkinMap[m_sessionId];
        }
        float skinPosY = static_cast<float>(skinIndex) * utl::GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }

    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity))
    {
        uint32_t skinIndex = 0;
        if (m_playerSkinMap.find(m_sessionId) != m_playerSkinMap.end())
        {
            skinIndex = m_playerSkinMap[m_sessionId];
        }
        float skinPosY = static_cast<float>(skinIndex) * utl::GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }

    auto beginSoundEntity = registry.createEntity()
                                .with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 1.0F, false, false)
                                .build();
    if (auto *audioComp = registry.getComponent<ecs::Audio>(beginSoundEntity))
    {
        audioComp->play = true;
    }
    m_beginSoundEntity = beginSoundEntity;

    m_stageManager = std::make_unique<StageManager>();

    // Preload all common textures to avoid lag spikes during gameplay
    preloadCommonTextures();

    setupEventSubscriptions();
}

void gme::GameMulti::preloadCommonTextures()
{
    // Preload all textures that will be used during gameplay
    // This prevents lag when entities are first created from world state

    std::vector<std::pair<std::string, std::string>> texturesToLoad = {
        {"player", utl::Path::Texture::TEXTURE_PLAYER},
        {"enemy_basic", utl::Path::Texture::TEXTURE_ENEMY_BASIC},
        {"enemy_advanced", utl::Path::Texture::TEXTURE_ENEMY_ADVANCED},
        {"boss", utl::Path::Texture::TEXTURE_BOSS},
        {"shoot", utl::Path::Texture::TEXTURE_SHOOT},
        {"shoot_charged", utl::Path::Texture::TEXTURE_SHOOT_CHARGED},
        {"enemy_projectile", utl::Path::Texture::TEXTURE_ENEMY_PROJECTILE},
        {"explosion", utl::Path::Texture::TEXTURE_EXPLOSION}};

    for (const auto &[id, path] : texturesToLoad)
    {
        if (m_loadedTextures.find(id) == m_loadedTextures.end())
        {
            m_renderer->createTexture(id, path);
            m_loadedTextures.insert(id);
        }
    }
}

void gme::GameMulti::setupEventSubscriptions() const
{
    utl::EventBus &eventBus = utl::EventBus::getInstance();
    eventBus.registerComponent(m_eventComponentId, "GameMulti");
    eventBus.subscribe(m_eventComponentId, utl::EventType::GAME_START);
    eventBus.subscribe(m_eventComponentId, utl::EventType::PLAYER_INPUT_RECEIVED);
    eventBus.subscribe(m_eventComponentId, utl::EventType::WORLD_STATE_RECEIVED);
    eventBus.subscribe(m_eventComponentId, utl::EventType::GAME_OVER);
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
            case utl::EventType::GAME_OVER:
            {
                utl::Logger::log("GameMulti: Received GAME_OVER event", utl::LogLevel::INFO);
                if (onGameOver)
                {
                    onGameOver();
                }
                break;
            }
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

        // Update local player score from world state
        for (const auto &entityState : worldState.entities)
        {
            if (entityState.id == m_sessionId && entityState.type == static_cast<uint16_t>(rnp::EntityType::PLAYER))
            {
                // Update the Score component with the server's score
                for (auto &[entity, score] : registry.getAll<ecs::Score>())
                {
                    score.value = static_cast<int>(entityState.score);
                    break;
                }
            }
        }

        // Track entities present in this world state
        std::set<uint32_t> currentEnemyIds;
        std::set<uint32_t> currentProjectileIds;

        if (m_firstWorldState)
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

            m_firstWorldState = false;
        }

        // First pass: collect IDs of entities in the world state
        for (const auto &entityState : worldState.entities)
        {
            if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::ENEMY) ||
                entityState.type == static_cast<std::uint16_t>(rnp::EntityType::BOSS))
            {
                currentEnemyIds.insert(entityState.id);
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PROJECTILE))
            {
                currentProjectileIds.insert(entityState.id);
            }
        }

        // Second pass: process entities
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

                // Synchronize health from server
                if (auto *health = registry.getComponent<ecs::Health>(m_localPlayerEntity))
                {
                    if (entityState.healthPercent != 255) // 255 means no health data
                    {
                        float previousHealth = health->current;
                        health->current = (static_cast<float>(entityState.healthPercent) / 100.0f) * health->max;

                        // Detect player death
                        if (health->current <= 0.0f && previousHealth > 0.0f)
                        {
                            utl::Logger::log("GameMulti: Local player died - disconnecting and showing game over",
                                             utl::LogLevel::WARNING);

                            // Send disconnect event to network
                            utl::EventBus &eventBus = utl::EventBus::getInstance();
                            std::vector<std::uint8_t> emptyData;
                            eventBus.publish(utl::EventType::REQUEST_DISCONNECT, emptyData, m_eventComponentId,
                                             utl::NETWORK_CLIENT);

                            // Trigger game over after a short delay to allow disconnect to process
                            if (onGameOver)
                            {
                                onGameOver();
                            }
                        }
                    }
                }

                // Synchronize beam charge from server
                if (auto *beamCharge = registry.getComponent<ecs::BeamCharge>(m_localPlayerEntity))
                {
                    // Always accept server charge value
                    beamCharge->current_charge = static_cast<float>(entityState.stateFlags) / 255.0f;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
            {
                if (!m_remotePlayers.contains(entityState.id))
                {
                    uint32_t skinIndex = 0;
                    if (m_playerSkinMap.contains(entityState.id))
                    {
                        skinIndex = m_playerSkinMap[entityState.id];
                    }
                    float skinPosY = static_cast<float>(skinIndex) * utl::GameConfig::Player::SPRITE_HEIGHT;

                    ecs::Entity remotePlayer =
                        registry.createEntity()
                            .with<ecs::Transform>("remote_player_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("remote_velocity_" + std::to_string(entityState.id), entityState.vx,
                                                 entityState.vy)
                            .with<ecs::Rect>("remote_rect_" + std::to_string(entityState.id), 0.F, skinPosY,
                                             static_cast<int>(utl::GameConfig::Player::SPRITE_WIDTH),
                                             static_cast<int>(utl::GameConfig::Player::SPRITE_HEIGHT))
                            .with<ecs::Scale>("remote_scale_" + std::to_string(entityState.id),
                                              utl::GameConfig::Player::SCALE, utl::GameConfig::Player::SCALE)
                            .with<ecs::Texture>("remote_texture_" + std::to_string(entityState.id),
                                                utl::Path::Texture::TEXTURE_PLAYER)
                            .with<ecs::Player>("remote_player_comp_" + std::to_string(entityState.id), false)
                            .with<ecs::Health>("remote_player_health_" + std::to_string(entityState.id), 100.0f, 100.0f)
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

                    // Update remote player health
                    if (auto *health = registry.getComponent<ecs::Health>(m_remotePlayers[entityState.id]))
                    {
                        if (entityState.healthPercent != 255)
                        {
                            health->current = (static_cast<float>(entityState.healthPercent) / 100.0f) * health->max;
                        }
                    }
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PROJECTILE))
            {

                if (!m_projectileEntities.contains(entityState.id))
                {
                    // Use subtype field to determine projectile type
                    bool isEnemyProjectile =
                        (entityState.subtype == static_cast<std::uint8_t>(rnp::EntitySubtype::PROJECTILE_ENEMY));
                    bool isSupercharged =
                        (entityState.subtype ==
                         static_cast<std::uint8_t>(rnp::EntitySubtype::PROJECTILE_PLAYER_SUPERCHARGED));

                    std::string texturePath;
                    float width, height, scale;

                    if (isEnemyProjectile)
                    {
                        // Enemy projectile
                        texturePath = utl::Path::Texture::TEXTURE_ENEMY_PROJECTILE;
                        width = 16.0f;
                        height = 16.0f;
                        scale = 1.0f;
                    }
                    else
                    {
                        // Player projectile
                        texturePath = isSupercharged ? utl::Path::Texture::TEXTURE_SHOOT_CHARGED
                                                     : utl::Path::Texture::TEXTURE_SHOOT;
                        width = isSupercharged ? 29.0f : 20.0f;
                        height = isSupercharged ? 24.0f : 10.0f;
                        scale = isSupercharged ? 1.5f : 1.0f;
                    }

                    auto entityBuilder =
                        registry.createEntity()
                            .with<ecs::Transform>("projectile_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("projectile_velocity_" + std::to_string(entityState.id),
                                                 entityState.vx, entityState.vy)
                            .with<ecs::Rect>("projectile_rect_" + std::to_string(entityState.id), 0.F, 0.F,
                                             static_cast<int>(width), static_cast<int>(height))
                            .with<ecs::Scale>("projectile_scale_" + std::to_string(entityState.id), scale, scale)
                            .with<ecs::Texture>("projectile_texture_" + std::to_string(entityState.id), texturePath);

                    if (isSupercharged && !isEnemyProjectile)
                    {
                        entityBuilder.with<ecs::Animation>("projectile_animation_" + std::to_string(entityState.id), 0,
                                                           4, 0.15f, 0.0f, 29, 24, 4);
                    }

                    ecs::Entity projectile = entityBuilder.build();
                    m_projectileEntities[entityState.id] = projectile;

                    // Don't play sound for every projectile to avoid audio spam and lag
                    // Sound should be played on player action locally, not on world state sync
                }
                else
                {
                    // Update existing projectile position and velocity
                    // Server handles cleanup of off-screen projectiles
                    if (auto *transform = registry.getComponent<ecs::Transform>(m_projectileEntities[entityState.id]))
                    {
                        transform->x = entityState.x;
                        transform->y = entityState.y;
                    }
                    if (auto *velocity = registry.getComponent<ecs::Velocity>(m_projectileEntities[entityState.id]))
                    {
                        velocity->x = entityState.vx;
                        velocity->y = entityState.vy;
                    }
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::ENEMY))
            {
                if (!m_enemyEntities.contains(entityState.id))
                {
                    // Use subtype field to determine enemy type
                    std::string texturePath;
                    float width, height, scale;
                    int animFrames = 4;

                    if (entityState.subtype == static_cast<std::uint8_t>(rnp::EntitySubtype::ENEMY_ADVANCED))
                    {
                        texturePath = utl::Path::Texture::TEXTURE_ENEMY_ADVANCED;
                        width = 33.0f;
                        height = 36.0f;
                        scale = 2.0f;
                    }
                    else // ENEMY_BASIC
                    {
                        texturePath = utl::Path::Texture::TEXTURE_ENEMY_BASIC;
                        width = 32.0f;
                        height = 32.0f;
                        scale = 2.0f;
                    }

                    ecs::Entity enemy =
                        registry.createEntity()
                            .with<ecs::Transform>("enemy_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("enemy_velocity_" + std::to_string(entityState.id), entityState.vx,
                                                 entityState.vy)
                            .with<ecs::Rect>("enemy_rect_" + std::to_string(entityState.id), 0.F, 0.F,
                                             static_cast<int>(width), static_cast<int>(height))
                            .with<ecs::Scale>("enemy_scale_" + std::to_string(entityState.id), scale, scale)
                            .with<ecs::Texture>("enemy_texture_" + std::to_string(entityState.id), texturePath)
                            .with<ecs::Animation>("enemy_animation_" + std::to_string(entityState.id), 0, animFrames,
                                                  0.5f, 0.0f, static_cast<int>(width), static_cast<int>(height),
                                                  animFrames)
                            .build();

                    m_enemyEntities[entityState.id] = enemy;

                    m_enemyData[entityState.id] = {.targetX = entityState.x,
                                                   .targetY = entityState.y,
                                                   .targetVx = entityState.vx,
                                                   .targetVy = entityState.vy,
                                                   .currentX = entityState.x,
                                                   .currentY = entityState.y,
                                                   .smoothFactor = ENEMY_SMOOTH_FACTOR,
                                                   .targetRotation = 0.0F,
                                                   .currentRotation = 0.0F};
                }
                else
                {
                    m_enemyData[entityState.id].targetX = entityState.x;
                    m_enemyData[entityState.id].targetY = entityState.y;
                    m_enemyData[entityState.id].targetVx = entityState.vx;
                    m_enemyData[entityState.id].targetVy = entityState.vy;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::BOSS))
            {
                if (!m_enemyEntities.contains(entityState.id))
                {
                    // Boss entity
                    std::string texturePath = utl::Path::Texture::TEXTURE_BOSS;
                    float width = 64.0f;
                    float height = 64.0f;
                    float scale = 3.0f;

                    ecs::Entity boss =
                        registry.createEntity()
                            .with<ecs::Transform>("boss_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("boss_velocity_" + std::to_string(entityState.id), entityState.vx,
                                                 entityState.vy)
                            .with<ecs::Rect>("boss_rect_" + std::to_string(entityState.id), 0.F, 0.F,
                                             static_cast<int>(width), static_cast<int>(height))
                            .with<ecs::Scale>("boss_scale_" + std::to_string(entityState.id), scale, scale)
                            .with<ecs::Texture>("boss_texture_" + std::to_string(entityState.id), texturePath)
                            .with<ecs::Animation>("boss_animation_" + std::to_string(entityState.id), 0, 2, 0.3f, 0.0f,
                                                  static_cast<int>(width), static_cast<int>(height), 2)
                            .build();

                    m_enemyEntities[entityState.id] = boss;

                    m_enemyData[entityState.id] = {.targetX = entityState.x,
                                                   .targetY = entityState.y,
                                                   .targetVx = entityState.vx,
                                                   .targetVy = entityState.vy,
                                                   .currentX = entityState.x,
                                                   .currentY = entityState.y,
                                                   .smoothFactor = ENEMY_SMOOTH_FACTOR,
                                                   .targetRotation = 0.0f,
                                                   .currentRotation = 0.0f};

                    utl::Logger::log("GameMulti: Boss spawned with ID " + std::to_string(entityState.id),
                                     utl::LogLevel::INFO);
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

        // Clean up entities that are no longer in the world state
        // Remove enemies that disappeared (create explosion before removal)
        std::vector<uint32_t> enemiesToRemove;
        for (const auto &enemyId : m_enemyEntities | std::views::keys)
        {
            if (!currentEnemyIds.contains(enemyId))
            {
                enemiesToRemove.push_back(enemyId);
            }
        }

        for (uint32_t enemyId : enemiesToRemove)
        {
            ecs::Entity enemyEntity = m_enemyEntities[enemyId];

            // Create explosion at enemy position before removing
            if (auto *transform = registry.getComponent<ecs::Transform>(enemyEntity))
            {
                registry.createEntity()
                    .with<ecs::Transform>("explosion_transform", transform->x, transform->y, 0.0f)
                    .with<ecs::Rect>("explosion_rect", 0.0f, 0.0f,
                                     static_cast<int>(utl::GameConfig::Explosion::SPRITE_WIDTH),
                                     static_cast<int>(utl::GameConfig::Explosion::SPRITE_HEIGHT))
                    .with<ecs::Scale>("explosion_scale", utl::GameConfig::Explosion::SCALE,
                                      utl::GameConfig::Explosion::SCALE)
                    .with<ecs::Texture>("explosion_texture", utl::Path::Texture::TEXTURE_EXPLOSION)
                    .with<ecs::Explosion>(
                        "explosion", 0, utl::GameConfig::Explosion::ANIMATION_FRAMES,
                        utl::GameConfig::Explosion::ANIMATION_DURATION, 0.0f, utl::GameConfig::Explosion::SPRITE_WIDTH,
                        utl::GameConfig::Explosion::SPRITE_HEIGHT, utl::GameConfig::Explosion::FRAMES_PER_ROW,
                        utl::GameConfig::Explosion::LIFETIME, 0.0f)
                    .build();
            }

            // Remove all components
            if (registry.hasComponent<ecs::Transform>(enemyEntity))
                registry.removeComponent<ecs::Transform>(enemyEntity);
            if (registry.hasComponent<ecs::Velocity>(enemyEntity))
                registry.removeComponent<ecs::Velocity>(enemyEntity);
            if (registry.hasComponent<ecs::Rect>(enemyEntity))
                registry.removeComponent<ecs::Rect>(enemyEntity);
            if (registry.hasComponent<ecs::Scale>(enemyEntity))
                registry.removeComponent<ecs::Scale>(enemyEntity);
            if (registry.hasComponent<ecs::Texture>(enemyEntity))
                registry.removeComponent<ecs::Texture>(enemyEntity);
            if (registry.hasComponent<ecs::Animation>(enemyEntity))
                registry.removeComponent<ecs::Animation>(enemyEntity);

            m_enemyEntities.erase(enemyId);
            m_enemyData.erase(enemyId);
        }

        // Remove projectiles that disappeared
        std::vector<uint32_t> projectilesToRemove;
        for (const auto &projectileId : m_projectileEntities | std::views::keys)
        {
            if (!currentProjectileIds.contains(projectileId))
            {
                projectilesToRemove.push_back(projectileId);
            }
        }

        for (uint32_t projectileId : projectilesToRemove)
        {
            ecs::Entity projectileEntity = m_projectileEntities[projectileId];

            // Remove all components
            if (registry.hasComponent<ecs::Transform>(projectileEntity))
            {
                registry.removeComponent<ecs::Transform>(projectileEntity);
            }
            if (registry.hasComponent<ecs::Velocity>(projectileEntity))
            {
                registry.removeComponent<ecs::Velocity>(projectileEntity);
            }
            if (registry.hasComponent<ecs::Rect>(projectileEntity))
            {
                registry.removeComponent<ecs::Rect>(projectileEntity);
            }
            if (registry.hasComponent<ecs::Scale>(projectileEntity))
            {
                registry.removeComponent<ecs::Scale>(projectileEntity);
            }
            if (registry.hasComponent<ecs::Texture>(projectileEntity))
            {
                registry.removeComponent<ecs::Texture>(projectileEntity);
            }
            if (registry.hasComponent<ecs::Animation>(projectileEntity))
            {
                registry.removeComponent<ecs::Animation>(projectileEntity);
            }

            m_projectileEntities.erase(projectileId);
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

    if (m_hudSystem)
    {
        m_hudSystem->update(reg, dt);
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
            StageManager::stopScrolling(reg);
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
                                         float dt, ecs::Registry &registry)
{
    for (auto &[entityId, interpData] : dataMap)
    {
        if (&dataMap == &m_remotePlayerData && entityId == m_sessionId)
        {
            continue;
        }

        if (entityMap.contains(entityId))
        {
            const ecs::Entity entity = entityMap[entityId];
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
