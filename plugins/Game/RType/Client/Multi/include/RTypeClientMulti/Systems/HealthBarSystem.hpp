///
/// @file HealthBarSystem.hpp
/// @brief System for rendering health bars above entities in multiplayer mode
/// @details This system automatically renders health bars above all entities that have
///          a Health component. The health bars dynamically change color based on the
///          remaining health percentage and follow the entity's position.
/// @namespace gme
/// @author R-Type Team
/// @date 2025
///

#pragma once

#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include <memory>

namespace gme
{
    ///
    /// @class HealthBarSystem
    /// @brief ECS System that renders dynamic health bars above entities
    /// @details This system queries all entities with a Health component and renders
    ///          a visual health bar above each entity. Features include:
    ///          - Dynamic color based on health percentage (green -> yellow -> orange -> red)
    ///          - Positioned above entities with configurable offset
    ///          - Border and background for better visibility
    ///          - Automatic health percentage calculation
    ///
    /// The system is designed for multiplayer where multiple players and enemies
    /// need health indicators.
    /// @namespace gme
    ///
    class HealthBarSystem final : public ecs::ASystem
    {
        public:
            ///
            /// @brief Constructor
            /// @param renderer Shared pointer to the rendering interface
            ///
            explicit HealthBarSystem(const std::shared_ptr<eng::IRenderer> &renderer);

            ///
            /// @brief Destructor
            ///
            ~HealthBarSystem() override = default;

            ///
            /// @brief Deleted copy constructor (non-copyable)
            ///
            HealthBarSystem(const HealthBarSystem &) = delete;

            ///
            /// @brief Deleted copy assignment operator (non-copyable)
            ///
            HealthBarSystem &operator=(const HealthBarSystem &) = delete;

            ///
            /// @brief Deleted move constructor (non-movable)
            ///
            HealthBarSystem(HealthBarSystem &&) = delete;

            ///
            /// @brief Deleted move assignment operator (non-movable)
            ///
            HealthBarSystem &operator=(HealthBarSystem &&) = delete;

            ///
            /// @brief Update the health bar system (called each frame)
            /// @param registry ECS registry containing all entities
            /// @param dt Delta time since last frame (unused)
            /// @details Iterates through all entities with Health and Transform components
            ///          and renders a health bar above each one
            ///
            void update(ecs::Registry &registry, float dt) override;

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer; ///< Reference to the renderer for drawing

            // Health bar visual configuration
            static constexpr float BAR_WIDTH = 40.0f;       ///< Width of the health bar in pixels
            static constexpr float BAR_HEIGHT = 4.0f;       ///< Height of the health bar in pixels
            static constexpr float BAR_OFFSET_Y = -15.0f;   ///< Vertical offset above entity (negative = above)
            static constexpr float BORDER_THICKNESS = 1.0f; ///< Thickness of the border around the bar

            // Health bar color thresholds and values
            static constexpr eng::Color HEALTH_HIGH = {0, 255, 0, 255};     ///< Green color for >75% health
            static constexpr eng::Color HEALTH_MEDIUM = {255, 255, 0, 255}; ///< Yellow color for 50-75% health
            static constexpr eng::Color HEALTH_LOW = {255, 100, 0, 255};    ///< Orange color for 25-50% health
            static constexpr eng::Color HEALTH_CRITICAL = {255, 0, 0, 255}; ///< Red color for <25% health
            static constexpr eng::Color BACKGROUND = {50, 50, 50, 200};     ///< Dark gray background (semi-transparent)
            static constexpr eng::Color BORDER = {255, 255, 255, 255};      ///< White border color

            ///
            /// @brief Calculate health bar color based on health percentage
            /// @param healthPercent Health percentage (0.0 = dead, 1.0 = full health)
            /// @return Color for the health bar fill
            /// @details Color transitions:
            ///          - >75%: Green (HEALTH_HIGH)
            ///          - 50-75%: Yellow (HEALTH_MEDIUM)
            ///          - 25-50%: Orange (HEALTH_LOW)
            ///          - <25%: Red (HEALTH_CRITICAL)
            ///
            eng::Color getHealthColor(float healthPercent) const;

            ///
            /// @brief Render a health bar at specified position
            /// @param x X position (center of entity)
            /// @param y Y position (center of entity)
            /// @param currentHealth Current health value
            /// @param maxHealth Maximum health value
            /// @details Draws a layered health bar with:
            ///          1. White border
            ///          2. Dark gray background
            ///          3. Colored health fill (proportional to current/max health)
            ///
            void drawHealthBar(float x, float y, float currentHealth, float maxHealth);
    };

} // namespace gme
