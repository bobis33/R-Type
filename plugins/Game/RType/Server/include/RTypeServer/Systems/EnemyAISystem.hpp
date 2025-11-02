///
/// @file EnemyAISystem.hpp
/// @brief Server-side enemy AI and behavior system for R-Type
/// @details This file contains the enemy AI system that controls all enemy behaviors,
///          movement patterns, and combat logic on the game server. The system implements
///          various AI patterns including straight movement, sine waves, aggressive pursuit,
///          stationary shooting, and zigzag patterns. All enemy behavior is server-authoritative
///          to ensure fair and synchronized gameplay.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "RTypeServer/EntityManager.hpp"
#include <random>
#include <unordered_map>

namespace gme
{
    ///
    /// @enum EnemyBehavior
    /// @brief Enumeration of enemy movement and behavior patterns
    /// @details Defines the various AI behavior patterns available for enemies.
    ///          Each pattern provides different challenges and gameplay variety.
    ///
    enum class EnemyBehavior : uint8_t
    {
        STRAIGHT = 0,   ///< Move straight left at constant speed (basic behavior)
        SINE_WAVE = 1,  ///< Sine wave vertical oscillation pattern (intermediate)
        AGGRESSIVE = 2, ///< Actively pursue nearest player (advanced behavior)
        STATIONARY = 3, ///< Stay in place and shoot at players (turret-like)
        ZIGZAG = 4      ///< Zigzag diagonal movement pattern (erratic)
    };

    ///
    /// @class EnemyAISystem
    /// @brief Server-authoritative enemy AI system managing all enemy behaviors
    /// @details This ECS system controls all enemy logic on the game server, including:
    ///          - Multiple movement patterns (straight, sine wave, aggressive, zigzag, stationary)
    ///          - Per-enemy-type specialized AI behaviors
    ///          - Shooting logic with randomized timing
    ///          - Player tracking and pursuit
    ///          - Screen boundary clamping
    ///          - Per-entity timing for independent movement patterns
    ///
    /// The system processes three enemy types:
    /// - Basic enemies: Simple movement patterns, occasional shooting
    /// - Advanced enemies: More complex patterns, more aggressive
    /// - Boss enemies: Special behaviors, spread shot patterns, higher durability
    ///
    /// All AI logic runs server-side to maintain authoritative gameplay state.
    ///
    /// @namespace gme
    ///
    class EnemyAISystem final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor
            /// @param registry ECS registry containing all entities and components
            /// @param entityManager Entity manager for spawning projectiles and effects
            /// @details Initializes the AI system with random number generation for shooting patterns
            ///
            explicit EnemyAISystem(ecs::Registry &registry, EntityManager &entityManager);

            ///
            /// @brief Destructor
            ///
            ~EnemyAISystem() override = default;

            /// @brief Deleted copy constructor (non-copyable)
            EnemyAISystem(const EnemyAISystem &) = delete;
            /// @brief Deleted copy assignment operator (non-copyable)
            EnemyAISystem &operator=(const EnemyAISystem &) = delete;
            /// @brief Deleted move constructor (non-movable)
            EnemyAISystem(EnemyAISystem &&) = delete;
            /// @brief Deleted move assignment operator (non-movable)
            EnemyAISystem &operator=(EnemyAISystem &&) = delete;

            ///
            /// @brief Update the enemy AI system (called each frame)
            /// @param registry ECS registry containing all entities
            /// @param deltaTime Time elapsed since last frame (in seconds)
            /// @details Processes AI logic for all enemy entities including movement, shooting,
            ///          and behavior pattern updates
            ///
            void update(ecs::Registry &registry, float deltaTime) override;

            ///
            /// @brief Set global enemy aggressiveness multiplier
            /// @param value Aggressiveness factor (0.0-2.0, default 1.0)
            /// @details Higher values make enemies pursue players more aggressively.
            ///          Affects movement speed and shooting frequency.
            ///
            void setAggressiveness(float value) { m_aggressiveness = value; }

            ///
            /// @brief Get current aggressiveness setting
            /// @return Current aggressiveness multiplier
            ///
            float getAggressiveness() const { return m_aggressiveness; }

        private:
            ecs::Registry &m_registry;      ///< ECS registry reference
            EntityManager &m_entityManager; ///< Entity manager for spawning projectiles

            float m_aggressiveness; ///< Global aggressiveness multiplier (affects pursuit speed)
            std::mt19937 m_rng;     ///< Random number generator for shooting patterns
            std::uniform_real_distribution<float> m_shootChance; ///< Random distribution for shoot probability

            // Per-entity timing data to avoid static variable synchronization issues
            std::unordered_map<std::uint32_t, float> m_enemyMovementTimers; ///< Movement timing per enemy ID
            std::unordered_map<std::uint32_t, float> m_bossSpreadTimers;    ///< Boss spread shot timing per boss ID

            ///
            /// @brief Update AI logic for basic enemy type
            /// @param enemyId Network ID of the enemy
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @details Implements simple movement patterns and occasional shooting
            ///
            void updateBasicEnemyAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            ///
            /// @brief Update AI logic for advanced enemy type
            /// @param enemyId Network ID of the enemy
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @details Implements more complex movement patterns and frequent shooting
            ///
            void updateAdvancedEnemyAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            ///
            /// @brief Update AI logic for boss enemy type
            /// @param enemyId Network ID of the boss
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @details Implements special boss behaviors including spread shot patterns
            ///
            void updateBossAI(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            ///
            /// @brief Apply sine wave vertical movement pattern
            /// @param enemyId Network ID of the enemy (for independent timing)
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @param frequency Oscillation frequency in Hz (default: 2.0)
            /// @param amplitude Vertical oscillation amplitude in pixels (default: 50.0)
            /// @details Creates smooth vertical oscillation while moving left
            ///
            void applySineWaveMovement(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime,
                                       float frequency = 2.0f, float amplitude = 50.0f);

            ///
            /// @brief Apply aggressive pursuit movement toward nearest player
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @param speed Movement speed in pixels per second (default: 150.0)
            /// @details Enemy moves directly toward nearest player position
            ///
            void applyAggressiveMovement(ecs::Entity enemy, float deltaTime, float speed = 150.0f);

            ///
            /// @brief Apply zigzag diagonal movement pattern
            /// @param enemyId Network ID of the enemy (for independent timing)
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @details Creates erratic zigzag movement pattern
            ///
            void applyZigzagMovement(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            ///
            /// @brief Attempt to shoot projectile at player
            /// @param enemyId Network ID of the enemy
            /// @param enemy ECS entity handle
            /// @param deltaTime Time elapsed since last frame
            /// @details Checks shooting conditions and spawns projectile with random chance
            ///
            void tryShoot(std::uint32_t enemyId, ecs::Entity enemy, float deltaTime);

            ///
            /// @brief Check if enemy is able to shoot (cooldown and component checks)
            /// @param enemy ECS entity handle
            /// @return True if enemy can shoot
            /// @details Verifies entity has required components and shooting cooldown has elapsed
            ///
            bool canShoot(ecs::Entity enemy) const;

            ///
            /// @brief Find the nearest player entity to a given position
            /// @param x X coordinate to search from
            /// @param y Y coordinate to search from
            /// @return Entity handle of nearest player, or invalid entity if none found
            /// @details Used for aggressive AI targeting
            ///
            ecs::Entity findNearestPlayer(float x, float y) const;

            ///
            /// @brief Calculate distance to nearest player
            /// @param x X coordinate to measure from
            /// @param y Y coordinate to measure from
            /// @return Distance to nearest player in pixels, or large value if no players
            /// @details Used for distance-based behavior decisions
            ///
            float getDistanceToNearestPlayer(float x, float y) const;

            ///
            /// @brief Clamp enemy position to visible screen bounds
            /// @param enemy ECS entity handle
            /// @details Prevents enemies from moving off-screen vertically
            ///
            void clampToScreen(ecs::Entity enemy);
    };

} // namespace gme
