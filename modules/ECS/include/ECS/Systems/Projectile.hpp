///
/// @file Projectile.hpp
/// @brief This file contains the projectile system definition
/// @namespace ecs
///

#pragma once

#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace ecs
{

    ///
    /// @class ProjectileSystem
    /// @brief Class for projectile system
    /// @namespace ecs
    ///
    class ProjectileSystem final : public ASystem
    {
        public:
            explicit ProjectileSystem(const std::shared_ptr<eng::IRenderer> & /* renderer */) {}
            ~ProjectileSystem() override = default;

            ProjectileSystem(const ProjectileSystem &) = delete;
            ProjectileSystem &operator=(const ProjectileSystem &) = delete;
            ProjectileSystem(ProjectileSystem &&) = delete;
            ProjectileSystem &operator=(const ProjectileSystem &&) = delete;

            void update(Registry &registry, float dt) override
            {
                std::vector<Entity> entitiesToRemove;

                for (auto &[entity, projectile] : registry.getAll<Projectile>())
                {
                    projectile.current_lifetime += dt;
                    if (projectile.current_lifetime >= projectile.lifetime)
                    {
                        entitiesToRemove.push_back(entity);
                        continue;
                    }
                    auto *transform = registry.getComponent<Transform>(entity);

                    if (auto *velocity = registry.getComponent<Velocity>(entity);
                        (transform != nullptr) && (velocity != nullptr))
                    {
                        transform->x += velocity->x * dt;
                        transform->y += velocity->y * dt;
                    }
                }

                for (const auto &entity : entitiesToRemove)
                {
                    if (registry.hasComponent<Projectile>(entity))
                        registry.removeComponent<Projectile>(entity);
                    if (registry.hasComponent<Transform>(entity))
                        registry.removeComponent<Transform>(entity);
                    if (registry.hasComponent<Velocity>(entity))
                        registry.removeComponent<Velocity>(entity);
                    if (registry.hasComponent<Rect>(entity))
                        registry.removeComponent<Rect>(entity);
                    if (registry.hasComponent<Scale>(entity))
                        registry.removeComponent<Scale>(entity);
                    if (registry.hasComponent<Texture>(entity))
                        registry.removeComponent<Texture>(entity);
                    if (registry.hasComponent<Animation>(entity))
                        registry.removeComponent<Animation>(entity);
                }
            }

    }; // class ProjectileSystem
} // namespace ecs

