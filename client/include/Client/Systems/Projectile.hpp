///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class ProjectileSystem final : public eng::ASystem
    {
        public:
            explicit ProjectileSystem(eng::IRenderer & /* renderer */) {}
            ~ProjectileSystem() override = default;

            ProjectileSystem(const ProjectileSystem &) = delete;
            ProjectileSystem &operator=(const ProjectileSystem &) = delete;
            ProjectileSystem(ProjectileSystem &&) = delete;
            ProjectileSystem &operator=(ProjectileSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
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
                    auto *velocity = registry.getComponent<ecs::Velocity>(entity);

                    if (transform && velocity)
                    {
                        transform->x += velocity->x * dt;
                        transform->y += velocity->y * dt;
                    }
                }

                for (const auto &entity : entitiesToRemove)
                {
                    if (registry.hasComponent<ecs::Projectile>(entity))
                        registry.removeComponent<ecs::Projectile>(entity);
                    if (registry.hasComponent<ecs::Transform>(entity))
                        registry.removeComponent<ecs::Transform>(entity);
                    if (registry.hasComponent<ecs::Velocity>(entity))
                        registry.removeComponent<ecs::Velocity>(entity);
                    if (registry.hasComponent<ecs::Rect>(entity))
                        registry.removeComponent<ecs::Rect>(entity);
                    if (registry.hasComponent<ecs::Scale>(entity))
                        registry.removeComponent<ecs::Scale>(entity);
                    if (registry.hasComponent<ecs::Texture>(entity))
                        registry.removeComponent<ecs::Texture>(entity);
                    if (registry.hasComponent<ecs::Animation>(entity))
                        registry.removeComponent<ecs::Animation>(entity);
                }
            }

    }; // class ProjectileSystem
} // namespace cli