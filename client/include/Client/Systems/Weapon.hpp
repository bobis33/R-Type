///
/// @file WeaponSystem.hpp
/// @brief Handles weapon firing logic
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

#include "ECS/Interfaces/ISystems.hpp"

namespace cli
{
    ///
    /// @class WeaponSystem
    /// @brief Manages weapon firing and charging
    /// @namespace cli
    ///
    class WeaponSystem final : public eng::ASystem
    {
        public:
            explicit WeaponSystem(const std::shared_ptr<eng::IRenderer> & /* renderer */) {}
            ~WeaponSystem() override = default;

            WeaponSystem(const WeaponSystem &) = delete;
            WeaponSystem &operator=(const WeaponSystem &) = delete;
            WeaponSystem(WeaponSystem &&) = delete;
            WeaponSystem &operator=(WeaponSystem &&) = delete;

            ///
            /// @brief Update weapon system
            /// @param registry The ECS registry
            /// @param dt Delta time
            ///
            void update(ecs::Registry &registry, float dt) override;

            ///
            /// @brief Reset weapon state
            ///
            void reset();

        private:
            float m_fireCooldown = 0.0f;
            bool m_isCharging = false;

            ///
            /// @brief Try to fire basic projectile
            /// @param registry The ECS registry
            /// @param x X position
            /// @param y Y position
            /// @return True if fired successfully
            ///
            bool tryFireBasic(ecs::Registry &registry, float x, float y);

            ///
            /// @brief Try to fire supercharged projectile
            /// @param registry The ECS registry
            /// @param x X position
            /// @param y Y position
            /// @return True if fired successfully
            ///
            bool tryFireSupercharged(ecs::Registry &registry, float x, float y);

            ///
            /// @brief Show loading animation in front of the player
            /// @param registry The ECS registry
            /// @param playerEntity The player entity
            /// @param playerTransform The player transform
            ///
            void showLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity,
                                      const ecs::Transform *playerTransform);

            ///
            /// @brief Hide loading animation
            /// @param registry The ECS registry
            /// @param playerEntity The player entity
            ///
            void hideLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity);
    }; // class WeaponSystem
} // namespace cli
