#include "RTypeClientMulti/Scenes/GameMulti.hpp"
#include "ECS/Component.hpp"
#include "Interfaces/IAudio.hpp"
#include "RTypeClientMulti/Managers/StageManager.hpp"
#include "RTypeClientMulti/Systems/PlayerControllerMulti.hpp"
#include "RTypeShared/GameConfig.hpp"
#include "Utils/Common.hpp"
#include "Utils/EventBus.hpp"
#include <algorithm>
#include <set>

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

    if (auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity))
    {
        uint32_t playerSkinIndex = 0;
        if (m_playerSkinMap.find(m_sessionId) != m_playerSkinMap.end())
        {
            playerSkinIndex = m_playerSkinMap[m_sessionId];
        }
        float skinPosY = static_cast<float>(playerSkinIndex) * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }

    auto beginSoundEntity = registry.createEntity()
                                .with<ecs::Audio>("game_begin", utl::Path::Audio::AUDIO_BEGIN, 1.0F, false, false)
                                .build();
    if (auto *audioComp = registry.getComponent<ecs::Audio>(beginSoundEntity))
    {
        audioComp->play = true;
    }

    m_stageManager = std::make_unique<StageManager>();

    setupEventSubscriptions();
}

void gme::GameMulti::setupEventSubscriptions()
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
    std::vector<utl::Event> events = eventBus.consumeForTarget(m_eventComponentId);
    for (const auto &event : events)
    {
        switch (event.type)
        {
            case utl::EventType::GAME_START:
            {
                break;
            }
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

        if (worldState.gameOver)
        {
            utl::Logger::log("GameMulti: Game Over received from server!", utl::LogLevel::INFO);
            if (onGameOver)
            {
                onGameOver();
            }
            return;
        }

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

                // Synchronize beam charge from server
                if (auto *beamCharge = registry.getComponent<ecs::BeamCharge>(m_localPlayerEntity))
                {
                    // Always accept server charge value
                    beamCharge->current_charge = static_cast<float>(entityState.stateFlags) / 255.0f;
                }
            }
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
            {
                if (m_remotePlayers.find(entityState.id) == m_remotePlayers.end())
                {
                    uint32_t remoteSkinIndex = 0;
                    if (m_playerSkinMap.find(entityState.id) != m_playerSkinMap.end())
                    {
                        remoteSkinIndex = m_playerSkinMap[entityState.id];
                    }
                    float skinPosY = static_cast<float>(remoteSkinIndex) * GameConfig::Player::SPRITE_HEIGHT;

                    ecs::Entity remotePlayer =
                        registry.createEntity()
                            .with<ecs::Transform>("remote_player_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("remote_velocity_" + std::to_string(entityState.id), entityState.vx,
                                                 entityState.vy)
                            .with<ecs::Rect>("remote_rect_" + std::to_string(entityState.id), 0.F, skinPosY,
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
                    bool isSupercharged = (entityState.vx > 1000.0f || std::abs(entityState.vx) > 1000.0f);
                    std::string texturePath =
                        isSupercharged ? utl::Path::Texture::TEXTURE_SHOOT_CHARGED : utl::Path::Texture::TEXTURE_SHOOT;

                    auto entityBuilder =
                        registry.createEntity()
                            .with<ecs::Transform>("projectile_" + std::to_string(entityState.id), entityState.x,
                                                  entityState.y, 0.F)
                            .with<ecs::Velocity>("projectile_velocity_" + std::to_string(entityState.id),
                                                 entityState.vx, entityState.vy)
                            .with<ecs::Rect>("projectile_rect_" + std::to_string(entityState.id), 0.F, 0.F,
                                             isSupercharged ? 29 : 20, isSupercharged ? 24 : 10)
                            .with<ecs::Scale>("projectile_scale_" + std::to_string(entityState.id),
                                              isSupercharged ? 1.5f : 1.0f, isSupercharged ? 1.5f : 1.0f)
                            .with<ecs::Texture>("projectile_texture_" + std::to_string(entityState.id), texturePath);

                    if (isSupercharged)
                    {
                        entityBuilder.with<ecs::Animation>("projectile_animation_" + std::to_string(entityState.id), 0,
                                                           4, 0.15f, 0.0f, 29, 24, 4);
                    }

                    ecs::Entity projectile = entityBuilder.build();
                    m_projectileEntities[entityState.id] = projectile;

                    auto shootSound =
                        registry.createEntity()
                            .with<ecs::Audio>("projectile_shoot_" + std::to_string(entityState.id),
                                              utl::Path::Audio::AUDIO_SUPERCHARGED_SHOT, 1.5F, false, false)
                            .build();
                    if (auto *audioComp = registry.getComponent<ecs::Audio>(shootSound))
                    {
                        audioComp->play = true;
                    }
                }
                else
                {
                    if (auto *transform = registry.getComponent<ecs::Transform>(m_projectileEntities[entityState.id]))
                    {
                        transform->x = entityState.x;
                        transform->y = entityState.y;

                        if (transform->x > 2000.0f || transform->x < -100.0f)
                        {
                            registry.removeComponent<ecs::Transform>(m_projectileEntities[entityState.id]);
                            registry.removeComponent<ecs::Velocity>(m_projectileEntities[entityState.id]);
                            registry.removeComponent<ecs::Rect>(m_projectileEntities[entityState.id]);
                            registry.removeComponent<ecs::Scale>(m_projectileEntities[entityState.id]);
                            registry.removeComponent<ecs::Texture>(m_projectileEntities[entityState.id]);
                            registry.removeComponent<ecs::Animation>(m_projectileEntities[entityState.id]);
                            m_projectileEntities.erase(entityState.id);
                        }
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
            else if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::BOSS) &&
                     entityState.stateFlags == 0xFF)
            {
                if (m_explosionEntities.find(entityState.id) == m_explosionEntities.end())
                {
                    ecs::Entity explosion =
                        registry.createEntity()
                            .with<ecs::Transform>("explosion_transform_" + std::to_string(entityState.id),
                                                  entityState.x, entityState.y, 0.F)
                            .with<ecs::Rect>("explosion_rect_" + std::to_string(entityState.id), 0.F, 0.F,
                                             static_cast<int>(GameConfig::Explosion::SPRITE_WIDTH),
                                             static_cast<int>(GameConfig::Explosion::SPRITE_HEIGHT))
                            .with<ecs::Scale>("explosion_scale_" + std::to_string(entityState.id),
                                              GameConfig::Explosion::SCALE, GameConfig::Explosion::SCALE)
                            .with<ecs::Texture>("explosion_texture_" + std::to_string(entityState.id),
                                                utl::Path::Texture::TEXTURE_EXPLOSION)
                            .with<ecs::Explosion>(
                                "explosion_" + std::to_string(entityState.id), 0,
                                GameConfig::Explosion::ANIMATION_FRAMES, GameConfig::Explosion::ANIMATION_DURATION,
                                0.0f, GameConfig::Explosion::SPRITE_WIDTH, GameConfig::Explosion::SPRITE_HEIGHT,
                                GameConfig::Explosion::FRAMES_PER_ROW, GameConfig::Explosion::LIFETIME, 0.0f)
                            .build();

                    m_explosionEntities[entityState.id] = explosion;
                    utl::Logger::log("GameMulti: Created explosion entity " + std::to_string(entityState.id),
                                     utl::LogLevel::INFO);
                }
            }
        }

        // Detect and remove players that are no longer in the world state
        std::set<uint32_t> currentPlayers;
        for (const auto &entityState : worldState.entities)
        {
            if (entityState.type == static_cast<std::uint16_t>(rnp::EntityType::PLAYER))
            {
                currentPlayers.insert(entityState.id);
            }
        }

        // Remove remote players that have disappeared
        std::vector<uint32_t> playersToRemove;
        for (const auto &[playerId, playerEntity] : m_remotePlayers)
        {
            if (currentPlayers.find(playerId) == currentPlayers.end())
            {
                playersToRemove.push_back(playerId);
            }
        }

        for (uint32_t playerId : playersToRemove)
        {
            ecs::Entity playerEntity = m_remotePlayers[playerId];

            registry.removeComponent<ecs::Transform>(playerEntity);
            registry.removeComponent<ecs::Velocity>(playerEntity);
            registry.removeComponent<ecs::Rect>(playerEntity);
            registry.removeComponent<ecs::Scale>(playerEntity);
            registry.removeComponent<ecs::Texture>(playerEntity);
            registry.removeComponent<ecs::Player>(playerEntity);

            m_remotePlayers.erase(playerId);
            m_remotePlayerData.erase(playerId);

            utl::Logger::log("GameMulti: Removed dead remote player " + std::to_string(playerId), utl::LogLevel::INFO);
        }

        if (currentPlayers.find(m_sessionId) == currentPlayers.end() && !m_localPlayerDied)
        {
            m_localPlayerDied = true;
            utl::Logger::log("GameMulti: Local player died!", utl::LogLevel::INFO);

            if (auto *texture = registry.getComponent<ecs::Texture>(m_localPlayerEntity))
            {
                registry.removeComponent<ecs::Texture>(m_localPlayerEntity);
                utl::Logger::log("GameMulti: Removed local player texture", utl::LogLevel::INFO);
            }

            // Remove BeamCharge component to stop beam bar from rendering
            if (registry.hasComponent<ecs::BeamCharge>(m_localPlayerEntity))
            {
                registry.removeComponent<ecs::BeamCharge>(m_localPlayerEntity);
                utl::Logger::log("GameMulti: Removed BeamCharge - beam bar will no longer render", utl::LogLevel::INFO);
            }

            auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
            if (!loadingEntities.empty())
            {
                std::vector<ecs::Entity> loadingToRemove;
                for (const auto &[entity, animation] : loadingEntities)
                {
                    loadingToRemove.push_back(entity);
                }
                for (const auto entity : loadingToRemove)
                {
                    registry.removeComponent<ecs::Transform>(entity);
                    registry.removeComponent<ecs::Rect>(entity);
                    registry.removeComponent<ecs::Scale>(entity);
                    registry.removeComponent<ecs::Texture>(entity);
                    registry.removeComponent<ecs::LoadingAnimation>(entity);
                }
                utl::Logger::log("GameMulti: Removed LoadingAnimation entities", utl::LogLevel::INFO);
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
    auto *playerRect = registry.getComponent<ecs::Rect>(m_localPlayerEntity);

    if (playerRect != nullptr)
    {
        const float skinPosY = static_cast<float>(m_skinIndex) * GameConfig::Player::SPRITE_HEIGHT;
        playerRect->pos_y = skinPosY;
    }
}

void gme::GameMulti::setScrollingSystem(gme::ScrollingSystem *scrollingSystem)
{
    if (scrollingSystem && m_stageManager)
    {
        scrollingSystem->setStageManager(m_stageManager.get());
    }
}
