#include "RTypeClientMulti/Systems/Projectile.hpp"
#include "ECS/Component.hpp"

void gme::ProjectileSystem::update(ecs::Registry &registry, float dt)
{
    std::vector<ecs::Entity> entitiesToRemove;

    for (auto &[entity, projectile] : registry.getAll<ecs::Projectile>())
    {
        projectile.current_lifetime += dt;
        if (projectile.current_lifetime >= projectile.lifetime)
        {
            entitiesToRemove.push_back(entity);
            continue;
        }
        auto *transform = registry.getComponent<ecs::Transform>(entity);

        if (const auto *velocity = registry.getComponent<ecs::Velocity>(entity);
            (transform != nullptr) && (velocity != nullptr))
        {
            transform->x += velocity->x * dt;
            transform->y += velocity->y * dt;
        }
    }

    for (const auto &entity : entitiesToRemove)
    {
        if (registry.hasComponent<ecs::Projectile>(entity))
        {
            registry.removeComponent<ecs::Projectile>(entity);
        }
        if (registry.hasComponent<ecs::Transform>(entity))
        {
            registry.removeComponent<ecs::Transform>(entity);
        }
        if (registry.hasComponent<ecs::Velocity>(entity))
        {
            registry.removeComponent<ecs::Velocity>(entity);
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
        if (registry.hasComponent<ecs::Animation>(entity))
        {
            registry.removeComponent<ecs::Animation>(entity);
        }
    }
}
