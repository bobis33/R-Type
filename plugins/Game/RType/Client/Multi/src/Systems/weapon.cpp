#include "RTypeClientMulti/Systems/Weapon.hpp"
#include "RTypeShared/GameConfig.hpp"
#include "Utils/Common.hpp"

void gme::WeaponSystem::update(ecs::Registry &registry, const float dt)
{
    using namespace GameConfig::Projectile;
    using namespace GameConfig::Beam;

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
            beamCharge->current_charge = 0.0f;
        }
    }
}

void gme::WeaponSystem::showLoadingAnimation(ecs::Registry &registry, ecs::Entity /* playerEntity */,
                                             const ecs::Transform *playerTransform)
{
    using namespace GameConfig::LoadingAnimation;

    auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
    for (const auto &[entity, animation] : loadingEntities)
    {
        if (auto *loadingTransform = registry.getComponent<ecs::Transform>(entity))
        {
            loadingTransform->x = playerTransform->x + OFFSET_X;
            loadingTransform->y = playerTransform->y + OFFSET_Y;
            return;
        }
    }

    (void) registry.createEntity()
                             .with<ecs::Transform>("loading_transform", playerTransform->x + OFFSET_X,
                                                   playerTransform->y + OFFSET_Y, 0.0f)
                             .with<ecs::Rect>("loading_rect", 0.0f, 0.0f, static_cast<int>(SPRITE_WIDTH),
                                              static_cast<int>(SPRITE_HEIGHT))
                             .with<ecs::Scale>("loading_scale", 1.0f, 1.0f)
                             .with<ecs::Texture>("loading_texture", utl::Path::Texture::TEXTURE_SHOOT_LOADING)
                             .with<ecs::LoadingAnimation>("loading_animation", 0, ANIMATION_FRAMES, ANIMATION_DURATION,
                                                          0.0f, SPRITE_WIDTH, SPRITE_HEIGHT, ANIMATION_FRAMES)
                             .build();
}

void gme::WeaponSystem::hideLoadingAnimation(ecs::Registry &registry, ecs::Entity /* playerEntity */)
{
    auto loadingEntities = registry.getAll<ecs::LoadingAnimation>();
    std::vector<ecs::Entity> toRemove;

    for (const auto &[entity, animation] : loadingEntities)
    {
        toRemove.push_back(entity);
    }

    for (const auto entity : toRemove)
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

