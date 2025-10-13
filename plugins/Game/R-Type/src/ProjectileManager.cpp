///
/// @file ProjectileManager.cpp
/// @brief Implementation of ProjectileManager
/// @namespace gme
///

#include "RTypeGame/ProjectileManager.hpp"
#include "RTypeGame/Common.hpp"
#include "ECS/Registry.hpp"

namespace gme
{
    ecs::Entity ProjectileManager::createBasicProjectile(ecs::Registry &registry, float x, float y, float velocityX,
                                                         float velocityY)
    {
        return createProjectile(registry, ecs::Projectile::BASIC, x, y, velocityX, velocityY);
    }

    ecs::Entity ProjectileManager::createSuperchargedProjectile(ecs::Registry &registry, float x, float y,
                                                                float velocityX, float velocityY)
    {
        return createProjectile(registry, ecs::Projectile::SUPERCHARGED, x, y, velocityX, velocityY);
    }

    ecs::Entity ProjectileManager::createProjectile(ecs::Registry &registry, ecs::Projectile::Type type, float x,
                                                    float y, float velocityX, float velocityY)
    {
        using namespace GameConfig::Projectile;

        if (type == ecs::Projectile::BASIC)
        {
            return registry.createEntity()
                .with<ecs::Transform>("projectile_transform", x, y, 0.F)
                .with<ecs::Velocity>("projectile_velocity", velocityX, velocityY)
                .with<ecs::Rect>("projectile_rect", 0.F, 0.F, static_cast<int>(Basic::SPRITE_WIDTH),
                                 static_cast<int>(Basic::SPRITE_HEIGHT))
                .with<ecs::Scale>("projectile_scale", Basic::SCALE, Basic::SCALE)
                .with<ecs::Texture>("projectile_texture", Path::Texture::TEXTURE_SHOOT)
                .with<ecs::Projectile>("projectile", type, Basic::DAMAGE, Basic::LIFETIME, 0.0f)
                .with<ecs::Hitbox>("projectile_hitbox", GameConfig::Hitbox::PROJECTILE_BASIC_RADIUS)
                .build();
        }
        else
        {
            return registry.createEntity()
                .with<ecs::Transform>("projectile_transform", x, y, 0.F)
                .with<ecs::Velocity>("projectile_velocity", velocityX, velocityY)
                .with<ecs::Rect>("projectile_rect", 0.F, 0.F, static_cast<int>(Supercharged::SPRITE_WIDTH),
                                 static_cast<int>(Supercharged::SPRITE_HEIGHT))
                .with<ecs::Scale>("projectile_scale", Supercharged::SCALE, Supercharged::SCALE)
                .with<ecs::Texture>("projectile_texture", Path::Texture::TEXTURE_SHOOT_CHARGED)
                .with<ecs::Projectile>("projectile", type, Supercharged::DAMAGE, Supercharged::LIFETIME, 0.0f)
                .with<ecs::Animation>("projectile_animation", 0, Supercharged::ANIMATION_FRAMES,
                                      Supercharged::ANIMATION_DURATION, 0.0f,
                                      static_cast<int>(Supercharged::SPRITE_WIDTH),
                                      static_cast<int>(Supercharged::SPRITE_HEIGHT), Supercharged::ANIMATION_FRAMES)
                .with<ecs::Hitbox>("projectile_hitbox", GameConfig::Hitbox::PROJECTILE_SUPERCHARGED_RADIUS)
                .build();
        }
    }
} // namespace gme

