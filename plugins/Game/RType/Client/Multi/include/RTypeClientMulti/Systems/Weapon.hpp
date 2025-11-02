///
/// @file Weapon.hpp
/// @brief Weapon system for R-Type multiplayer client
/// @details This file contains the weapon system that manages weapon firing, charging mechanics,
///          and visual feedback for the multiplayer client. The system handles client-side
///          weapon animations and effects while the actual projectile spawning is managed
///          by the server for authoritative gameplay.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
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
    /// @brief ECS system that manages weapon charging and visual effects in multiplayer
    /// @details This system handles client-side weapon mechanics including:
    ///          - Weapon charge animation (for charged shots)
    ///          - Loading/charging visual indicators
    ///          - Fire cooldown management
    ///          - Audio feedback for charged shots
    ///
    /// In multiplayer mode, actual projectile creation is server-authoritative.
    /// This system only provides visual and audio feedback to the player.
    ///
    /// @namespace gme
    ///
    class WeaponSystem final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor
            /// @param renderer Shared pointer to the renderer (unused in current implementation)
            ///
            explicit WeaponSystem(const std::shared_ptr<eng::IRenderer> & /* renderer */) {}

            ///
            /// @brief Destructor
            ///
            ~WeaponSystem() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            WeaponSystem(const WeaponSystem &) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            WeaponSystem &operator=(const WeaponSystem &) = delete;
            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            WeaponSystem(WeaponSystem &&) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            WeaponSystem &operator=(WeaponSystem &&) = delete;

            ///
            /// @brief Update weapon system (called each frame)
            /// @param registry ECS registry containing all entities and components
            /// @param dt Delta time since last frame (in seconds)
            /// @details Handles weapon charge animation state and cooldown timers.
            ///          Updates visual charging indicators based on player input state.
            ///
            void update(ecs::Registry &registry, float dt) override;

            ///
            /// @brief Reset weapon state to initial values
            /// @details Resets fire cooldown and charging state. Used when resetting game state.
            ///
            void reset()
            {
                m_fireCooldown = 0.0f;
                m_isCharging = false;
            }

        private:
            float m_fireCooldown = 0.0f; ///< Remaining cooldown time before next shot (seconds)
            bool m_isCharging = false;   ///< Whether weapon is currently charging

            ///
            /// @brief Display charging/loading animation in front of player
            /// @param registry ECS registry
            /// @param playerEntity Entity ID of the player
            /// @param playerTransform Pointer to player's transform component
            /// @details Creates or updates a visual indicator showing weapon charge progress
            ///
            static void showLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity,
                                             const ecs::Transform *playerTransform);

            ///
            /// @brief Hide/remove the charging animation
            /// @param registry ECS registry
            /// @param playerEntity Entity ID of the player
            /// @details Removes the visual charging indicator when charging is complete or cancelled
            ///
            static void hideLoadingAnimation(ecs::Registry &registry, ecs::Entity playerEntity);

            ///
            /// @brief Ensure super shot audio entity exists
            /// @param registry ECS registry
            /// @details Creates the audio entity for charged shot sound if it doesn't exist
            ///
            void ensureSuperShotAudio(ecs::Registry &registry);
            void ensureBasicShotAudio(ecs::Registry &registry);

            ecs::Entity m_superShotAudioEntity = ecs::INVALID_ENTITY;
            ecs::Entity m_basicShotAudioEntity = ecs::INVALID_ENTITY;
    }; // class WeaponSystem
} // namespace gme
