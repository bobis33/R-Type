#include <ranges>

#include "RTypeClientSolo/Managers/ProjectileManager.hpp"
#include "RTypeClientSolo/Systems/Weapon.hpp"
#include "Utils/Common.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

void gme::WeaponSystem::update(ecs::Registry &registry, const float dt)
{
    if (m_fireCooldown > 0.0f)
    {
        m_fireCooldown -= dt;
    }
    auto playerEntities = registry.getAll<ecs::Player>();
    if (playerEntities.empty())
        return;
    auto &[playerEntity, player] = *playerEntities.begin();
    const auto *transform = registry.getComponent<ecs::Transform>(playerEntity);
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
    float projectileX = transform->x + utl::GameConfig::Player::SPRITE_WIDTH;
    float projectileY = transform->y + utl::GameConfig::Player::SPRITE_HEIGHT / 2.0f;
    if (spacePressed)
    {
        if (!m_isCharging)
        {
            m_isCharging = true;
        }
        beamCharge->current_charge += utl::GameConfig::Beam::CHARGE_RATE * dt;
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
                if (const float chargeThreshold = beamCharge->max_charge * 0.5F;
                    beamCharge->current_charge >= chargeThreshold)
                {
                    if (tryFireSupercharged(registry, projectileX, projectileY))
                    {
                        beamCharge->current_charge = 0.0f;
                        m_fireCooldown = utl::GameConfig::Projectile::Supercharged::FIRE_COOLDOWN;
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

bool gme::WeaponSystem::tryFireBasic(ecs::Registry &registry, float x, float y)
{
    if (m_fireCooldown > 0.0f)
        return false;

    ProjectileManager::createBasicProjectile(registry, x, y, utl::GameConfig::Projectile::Basic::SPEED, 0.0f);
    m_fireCooldown = utl::GameConfig::Projectile::Basic::FIRE_COOLDOWN;
    return true;
}

bool gme::WeaponSystem::tryFireSupercharged(ecs::Registry &registry, float x, float y)
{
    ProjectileManager::createSuperchargedProjectile(registry, x, y, utl::GameConfig::Projectile::Supercharged::SPEED,
                                                    0.0f);
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

void gme::WeaponSystem::showLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity,
                                             const ecs::Transform *playerTransform)
{
    for (auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
         const auto &entity : loadingEntities | std::views::keys)
    {
        if (auto *loadingTransform = registry.getComponent<ecs::Transform>(entity))
        {
            loadingTransform->x = playerTransform->x + utl::GameConfig::LoadingAnimation::OFFSET_X;
            loadingTransform->y = playerTransform->y + utl::GameConfig::LoadingAnimation::OFFSET_Y;
            return;
        }
    }

    auto loadingEntity =
        registry.createEntity()
            .with<ecs::Transform>("loading_transform", playerTransform->x + utl::GameConfig::LoadingAnimation::OFFSET_X,
                                  playerTransform->y + utl::GameConfig::LoadingAnimation::OFFSET_Y, 0.0f)
            .with<ecs::Rect>("loading_rect", 0.0f, 0.0f,
                             static_cast<int>(utl::GameConfig::LoadingAnimation::SPRITE_WIDTH),
                             static_cast<int>(utl::GameConfig::LoadingAnimation::SPRITE_HEIGHT))
            .with<ecs::Scale>("loading_scale", 1.0f, 1.0f)
            .with<ecs::Texture>("loading_texture", utl::Path::Texture::TEXTURE_SHOOT_LOADING)
            .with<ecs::LoadingAnimation>("loading_animation", 0, utl::GameConfig::LoadingAnimation::ANIMATION_FRAMES,
                                         utl::GameConfig::LoadingAnimation::ANIMATION_DURATION, 0.0f,
                                         utl::GameConfig::LoadingAnimation::SPRITE_WIDTH,
                                         utl::GameConfig::LoadingAnimation::SPRITE_HEIGHT,
                                         utl::GameConfig::LoadingAnimation::ANIMATION_FRAMES)
            .build();
}

void gme::WeaponSystem::hideLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity)
{
    auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
    std::vector<ecs::Entity> toRemove;

    for (const auto &entity : loadingEntities | std::views::keys)
    {
        toRemove.push_back(entity);
    }

    for (const auto entity : toRemove)
    {
        if (registry.hasComponent<ecs::Transform>(entity))
        {
            registry.removeComponent<ecs::Transform>(entity);
        }
        if (registry.hasComponent<ecs::Rect>(entity))
        {
            registry.removeComponent<ecs::Rect>(entity);
        }
        if (registry.hasComponent<ecs::Scale>(entity))
        {
            registry.removeComponent<ecs::Scale>(entity);
        }
        if (registry.hasComponent<ecs::Texture>(entity))
        {
            registry.removeComponent<ecs::Texture>(entity);
        }
        if (registry.hasComponent<ecs::LoadingAnimation>(entity))
        {
            registry.removeComponent<ecs::LoadingAnimation>(entity);
        }
    }
}

void gme::WeaponSystem::ensureSuperShotAudio(ecs::Registry &registry)
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