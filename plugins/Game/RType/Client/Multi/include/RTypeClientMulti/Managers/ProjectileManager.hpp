///
/// @file ProjectileManager.hpp
/// @brief Manages projectile creation and configuration for multiplayer
/// @namespace gme
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"

namespace gme
{
    ///
    /// @class ProjectileManager
    /// @brief Handles projectile creation and management
    /// @namespace gme
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
    }; // class ProjectileManager
} // namespace gme

