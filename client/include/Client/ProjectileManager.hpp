///
/// @file ProjectileManager.hpp
/// @brief Manages projectile creation and configuration
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "GameConfig.hpp"

namespace cli
{
    ///
    /// @class ProjectileManager
    /// @brief Handles projectile creation and management
    /// @namespace cli
    ///
    class ProjectileManager
    {
        public:
            ProjectileManager() = default;
            ~ProjectileManager() = default;

            ProjectileManager(const ProjectileManager &) = delete;
            ProjectileManager &operator=(const ProjectileManager &) = delete;
            ProjectileManager(ProjectileManager &&) = delete;
            ProjectileManager &operator=(ProjectileManager &&) = delete;

            ///
            /// @brief Create a basic projectile
            /// @param registry The ECS registry
            /// @param x X position
            /// @param y Y position
            /// @param velocityX X velocity
            /// @param velocityY Y velocity
            /// @return The created entity
            ///
            static ecs::Entity createBasicProjectile(ecs::Registry &registry, float x, float y, float velocityX,
                                                     float velocityY);

            ///
            /// @brief Create a supercharged projectile
            /// @param registry The ECS registry
            /// @param x X position
            /// @param y Y position
            /// @param velocityX X velocity
            /// @param velocityY Y velocity
            /// @return The created entity
            ///
            static ecs::Entity createSuperchargedProjectile(ecs::Registry &registry, float x, float y, float velocityX,
                                                            float velocityY);

        private:
            ///
            /// @brief Create a projectile with given parameters
            /// @param registry The ECS registry
            /// @param type Projectile type
            /// @param x X position
            /// @param y Y position
            /// @param velocityX X velocity
            /// @param velocityY Y velocity
            /// @return The created entity
            ///
            static ecs::Entity createProjectile(ecs::Registry &registry, ecs::Projectile::Type type, float x, float y,
                                                float velocityX, float velocityY);
    }; // class ProjectileManager
} // namespace cli
