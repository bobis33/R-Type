///
/// @file HealthBarSystem.hpp
/// @brief System for rendering health bars above players
/// @namespace gme
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
    /// @brief Renders health bars above entities with Health component
    /// @namespace gme
    ///
    class HealthBarSystem final : public ecs::ASystem
    {
        public:
            explicit HealthBarSystem(const std::shared_ptr<eng::IRenderer> &renderer);
            ~HealthBarSystem() override = default;

            HealthBarSystem(const HealthBarSystem &) = delete;
            HealthBarSystem &operator=(const HealthBarSystem &) = delete;
            HealthBarSystem(HealthBarSystem &&) = delete;
            HealthBarSystem &operator=(HealthBarSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override;

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;

            // Health bar configuration
            static constexpr float BAR_WIDTH = 40.0f;
            static constexpr float BAR_HEIGHT = 4.0f;
            static constexpr float BAR_OFFSET_Y = -15.0f; // Above the player
            static constexpr float BORDER_THICKNESS = 1.0f;

            // Colors
            static constexpr eng::Color HEALTH_HIGH = {0, 255, 0, 255};     // Green
            static constexpr eng::Color HEALTH_MEDIUM = {255, 255, 0, 255}; // Yellow
            static constexpr eng::Color HEALTH_LOW = {255, 100, 0, 255};    // Orange
            static constexpr eng::Color HEALTH_CRITICAL = {255, 0, 0, 255}; // Red
            static constexpr eng::Color BACKGROUND = {50, 50, 50, 200};     // Dark gray
            static constexpr eng::Color BORDER = {255, 255, 255, 255};      // White

            /// @brief Get health bar color based on health percentage
            /// @param healthPercent Health percentage (0.0 - 1.0)
            /// @return Color for the health bar
            eng::Color getHealthColor(float healthPercent) const;

            /// @brief Draw a health bar for an entity
            /// @param x X position (center of entity)
            /// @param y Y position (center of entity)
            /// @param currentHealth Current health value
            /// @param maxHealth Maximum health value
            void drawHealthBar(float x, float y, float currentHealth, float maxHealth);
    };

} // namespace gme
