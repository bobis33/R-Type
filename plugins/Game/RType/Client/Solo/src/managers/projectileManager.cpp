#include "RTypeClientSolo/Managers/ProjectileManager.hpp"
#include "Utils/Common.hpp"
#include "Utils/HitboxUtils.hpp"
#include "Utils/RTypeShared/GameConfig.hpp"

namespace gme
{
    ecs::Entity ProjectileManager::createBasicProjectile(ecs::Registry &registry, const float x, const float y,
                                                         const float velocityX, const float velocityY)
    {
        return createProjectile(registry, ecs::Projectile::BASIC, x, y, velocityX, velocityY);
    }

    ecs::Entity ProjectileManager::createSuperchargedProjectile(ecs::Registry &registry, const float x, const float y,
                                                                const float velocityX, const float velocityY)
    {
        return createProjectile(registry, ecs::Projectile::SUPERCHARGED, x, y, velocityX, velocityY);
    }

    ecs::Entity ProjectileManager::createProjectile(ecs::Registry &registry, ecs::Projectile::Type type, float x,
                                                    float y, float velocityX, float velocityY)
    {
        if (type == ecs::Projectile::BASIC)
        {
            auto [offsetX, offsetY] = utl::calculateHitboxOffsets(utl::GameConfig::Projectile::Basic::SPRITE_WIDTH,
                                                                  utl::GameConfig::Projectile::Basic::SPRITE_HEIGHT,
                                                                  utl::GameConfig::Projectile::Basic::SCALE);

            return registry.createEntity()
                .with<ecs::Transform>("projectile_transform", x, y, 0.F)
                .with<ecs::Velocity>("projectile_velocity", velocityX, velocityY)
                .with<ecs::Rect>("projectile_rect", 0.F, 0.F,
                                 static_cast<int>(utl::GameConfig::Projectile::Basic::SPRITE_WIDTH),
                                 static_cast<int>(utl::GameConfig::Projectile::Basic::SPRITE_HEIGHT))
                .with<ecs::Scale>("projectile_scale", utl::GameConfig::Projectile::Basic::SCALE,
                                  utl::GameConfig::Projectile::Basic::SCALE)
                .with<ecs::Texture>("projectile_texture", utl::Path::Texture::TEXTURE_SHOOT)
                .with<ecs::Projectile>("projectile", type, utl::GameConfig::Projectile::Basic::DAMAGE,
                                       utl::GameConfig::Projectile::Basic::LIFETIME, 0.0f, 1)
                .with<ecs::Hitbox>("projectile_hitbox", utl::GameConfig::Hitbox::PROJECTILE_BASIC_RADIUS, offsetX,
                                   offsetY)
                .build();
        }

        auto [offsetX, offsetY] = utl::calculateHitboxOffsets(utl::GameConfig::Projectile::Supercharged::SPRITE_WIDTH,
                                                              utl::GameConfig::Projectile::Supercharged::SPRITE_HEIGHT,
                                                              utl::GameConfig::Projectile::Supercharged::SCALE);

        return registry.createEntity()
            .with<ecs::Transform>("projectile_transform", x, y, 0.F)
            .with<ecs::Velocity>("projectile_velocity", velocityX, velocityY)
            .with<ecs::Rect>("projectile_rect", 0.F, 0.F,
                             static_cast<int>(utl::GameConfig::Projectile::Supercharged::SPRITE_WIDTH),
                             static_cast<int>(utl::GameConfig::Projectile::Supercharged::SPRITE_HEIGHT))
            .with<ecs::Scale>("projectile_scale", utl::GameConfig::Projectile::Supercharged::SCALE,
                              utl::GameConfig::Projectile::Supercharged::SCALE)
            .with<ecs::Texture>("projectile_texture", utl::Path::Texture::TEXTURE_SHOOT_CHARGED)
            .with<ecs::Projectile>("projectile", type, utl::GameConfig::Projectile::Supercharged::DAMAGE,
                                   utl::GameConfig::Projectile::Supercharged::LIFETIME, 0.0f, 3)
            .with<ecs::Animation>("projectile_animation", 0,
                                  utl::GameConfig::Projectile::Supercharged::ANIMATION_FRAMES,
                                  utl::GameConfig::Projectile::Supercharged::ANIMATION_DURATION, 0.0f,
                                  static_cast<int>(utl::GameConfig::Projectile::Supercharged::SPRITE_WIDTH),
                                  static_cast<int>(utl::GameConfig::Projectile::Supercharged::SPRITE_HEIGHT),
                                  utl::GameConfig::Projectile::Supercharged::ANIMATION_FRAMES)
            .with<ecs::Hitbox>("projectile_hitbox", utl::GameConfig::Hitbox::PROJECTILE_SUPERCHARGED_RADIUS, offsetX,
                               offsetY)
            .build();
    }
} // namespace gme
