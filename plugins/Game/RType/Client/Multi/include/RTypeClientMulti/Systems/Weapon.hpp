///
/// @file Weapon.hpp
/// @brief Handles weapon firing logic for multiplayer
/// @namespace gme
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{
    ///
    /// @class WeaponSystem
    /// @brief Manages weapon firing and charging (client-side visual feedback only)
    /// @namespace gme
    ///
    class WeaponSystem final : public ecs::ASystem
    {
        public:
            explicit WeaponSystem(const std::shared_ptr<eng::IRenderer> & /* renderer */) {}
            ~WeaponSystem() override = default;

            WeaponSystem(const WeaponSystem &) = delete;
            WeaponSystem &operator=(const WeaponSystem &) = delete;
            WeaponSystem(WeaponSystem &&) = delete;
            WeaponSystem &operator=(WeaponSystem &&) = delete;

            ///
            /// @brief Update weapon system (handles charging animation)
            /// @param registry The ECS registry
            /// @param dt Delta time
            ///
            void update(ecs::Registry &registry, float dt) override;

            ///
            /// @brief Reset weapon state
            ///
            void reset()
            {
                m_fireCooldown = 0.0f;
                m_isCharging = false;
            }

        private:
            float m_fireCooldown = 0.0f;
            bool m_isCharging = false;

            ///
            /// @brief Show loading animation in front of the player
            /// @param registry The ECS registry
            /// @param playerEntity The player entity
            /// @param playerTransform The player transform
            ///
            static void showLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity,
                                             const ecs::Transform *playerTransform);

            ///
            /// @brief Hide loading animation
            /// @param registry The ECS registry
            /// @param playerEntity The player entity
            ///
            static void hideLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity);

            void ensureSuperShotAudio(ecs::Registry &registry);

            ecs::Entity m_superShotAudioEntity = ecs::INVALID_ENTITY;
    }; // class WeaponSystem
} // namespace gme
