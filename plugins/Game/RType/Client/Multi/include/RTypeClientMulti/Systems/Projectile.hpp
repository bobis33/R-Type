///
/// @file Projectile.hpp
/// @brief Projectile system for multiplayer
/// @namespace gme
///

#pragma once

#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    class ProjectileSystem final : public ecs::ASystem
    {
        public:
            explicit ProjectileSystem(const std::shared_ptr<eng::IRenderer> & /* renderer */) {}
            ~ProjectileSystem() override = default;

            ProjectileSystem(const ProjectileSystem &) = delete;
            ProjectileSystem &operator=(const ProjectileSystem &) = delete;
            ProjectileSystem(ProjectileSystem &&) = delete;
            ProjectileSystem &operator=(ProjectileSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override;

    }; // class ProjectileSystem
} // namespace gme

