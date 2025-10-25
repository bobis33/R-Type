#include "RTypeClientSolo/Systems/Weapon.hpp"
#include "RTypeClientSolo/GameConfig.hpp"
#include "RTypeClientSolo/Managers/ProjectileManager.hpp"
#include "Utils/Common.hpp"

namespace gme
{
    void WeaponSystem::update(ecs::Registry &registry, float dt)
    {
        using namespace GameConfig::Projectile;
        using namespace GameConfig::Beam;

        // Update cooldowns
        if (m_fireCooldown > 0.0f)
        {
            m_fireCooldown -= dt;
        }
        auto playerEntities = registry.getAll<ecs::Player>();
        if (playerEntities.empty())
            return;
        auto &[playerEntity, player] = *playerEntities.begin();
        auto *transform = registry.getComponent<ecs::Transform>(playerEntity);
        auto *beamCharge = registry.getComponent<ecs::BeamCharge>(playerEntity);
        if (!transform || !beamCharge)
            return;
        auto keyboardEntities = registry.getAll<ecs::KeyboardInput>();
        bool spacePressed = false;
        if (!keyboardEntities.empty())
        {
            auto &[keyboardEntity, keyboardInput] = *keyboardEntities.begin();
            spacePressed = keyboardInput.space_pressed;
        }
        float projectileX = transform->x + GameConfig::Player::SPRITE_WIDTH;
        float projectileY = transform->y + GameConfig::Player::SPRITE_HEIGHT / 2.0f;
        if (spacePressed)
        {
            if (!m_isCharging)
            {
                m_isCharging = true;
            }
            beamCharge->current_charge += CHARGE_RATE * dt;
            if (beamCharge->current_charge > beamCharge->max_charge)
                beamCharge->current_charge = beamCharge->max_charge;
            if (beamCharge->current_charge < beamCharge->max_charge)
            {
                showLoadingAnimation(registry, playerEntity, transform);
            }
            else
            {
                hideLoadingAnimation(registry, playerEntity);
            }
            return;
        }
        else
        {
            if (m_isCharging)
            {
                m_isCharging = false;
                hideLoadingAnimation(registry, playerEntity);
                if (m_fireCooldown <= 0.0f)
                {
                    float chargeThreshold = beamCharge->max_charge * 0.5f;
                    if (beamCharge->current_charge >= chargeThreshold)
                    {
                        if (tryFireSupercharged(registry, projectileX, projectileY))
                        {
                            beamCharge->current_charge = 0.0f;
                            m_fireCooldown = Supercharged::FIRE_COOLDOWN;
                        }
                    }
                    else
                    {
                        tryFireBasic(registry, projectileX, projectileY);
                    }
                }
            }
        }
    }

    void WeaponSystem::reset()
    {
        m_fireCooldown = 0.0f;
        m_isCharging = false;
    }

    bool WeaponSystem::tryFireBasic(ecs::Registry &registry, float x, float y)
    {
        using namespace GameConfig::Projectile;

        if (m_fireCooldown > 0.0f)
            return false;

        ProjectileManager::createBasicProjectile(registry, x, y, Basic::SPEED, 0.0f);
        m_fireCooldown = Basic::FIRE_COOLDOWN;
        return true;
    }

    bool WeaponSystem::tryFireSupercharged(ecs::Registry &registry, float x, float y)
    {
        using namespace GameConfig::Projectile;

        ProjectileManager::createSuperchargedProjectile(registry, x, y, Supercharged::SPEED, 0.0f);
        ensureSuperShotAudio(registry);
        if (m_superShotAudioEntity != ecs::INVALID_ENTITY)
        {
            if (auto *audio = registry.getComponent<ecs::Audio>(m_superShotAudioEntity))
            {
                audio->play = true;
            }
        }
        return true;
    }

    void WeaponSystem::showLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity,
                                            const ecs::Transform *playerTransform)
    {
        using namespace GameConfig::LoadingAnimation;

        auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
        for (auto &[entity, animation] : loadingEntities)
        {
            auto *loadingTransform = registry.getComponent<ecs::Transform>(entity);
            if (loadingTransform)
            {
                loadingTransform->x = playerTransform->x + OFFSET_X;
                loadingTransform->y = playerTransform->y + OFFSET_Y;
                return;
            }
        }

        auto loadingEntity =
            registry.createEntity()
                .with<ecs::Transform>("loading_transform", playerTransform->x + OFFSET_X, playerTransform->y + OFFSET_Y,
                                      0.0f)
                .with<ecs::Rect>("loading_rect", 0.0f, 0.0f, static_cast<int>(SPRITE_WIDTH),
                                 static_cast<int>(SPRITE_HEIGHT))
                .with<ecs::Scale>("loading_scale", 1.0f, 1.0f)
                .with<ecs::Texture>("loading_texture", utl::Path::Texture::TEXTURE_SHOOT_LOADING)
                .with<ecs::LoadingAnimation>("loading_animation", 0, ANIMATION_FRAMES, ANIMATION_DURATION, 0.0f,
                                             SPRITE_WIDTH, SPRITE_HEIGHT, ANIMATION_FRAMES)
                .build();
    }

    void WeaponSystem::hideLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity)
    {
        auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
        std::vector<ecs::Entity> toRemove;

        for (auto &[entity, animation] : loadingEntities)
        {
            toRemove.push_back(entity);
        }

        for (auto entity : toRemove)
        {
            if (registry.hasComponent<ecs::Transform>(entity))
                registry.removeComponent<ecs::Transform>(entity);
            if (registry.hasComponent<ecs::Rect>(entity))
                registry.removeComponent<ecs::Rect>(entity);
            if (registry.hasComponent<ecs::Scale>(entity))
                registry.removeComponent<ecs::Scale>(entity);
            if (registry.hasComponent<ecs::Texture>(entity))
                registry.removeComponent<ecs::Texture>(entity);
            if (registry.hasComponent<ecs::LoadingAnimation>(entity))
                registry.removeComponent<ecs::LoadingAnimation>(entity);
        }
    }

    void WeaponSystem::ensureSuperShotAudio(ecs::Registry &registry)
    {
        if (m_superShotAudioEntity != ecs::INVALID_ENTITY && registry.hasComponent<ecs::Audio>(m_superShotAudioEntity))
        {
            return;
        }

        m_superShotAudioEntity =
            registry.createEntity()
                .with<ecs::Audio>("player_super_shot", utl::Path::Audio::AUDIO_SUPERCHARGED_SHOT, 2.0F, false, false)
                .build();
    }
} // namespace gme
