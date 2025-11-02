///
/// @file HealthBarSystem.cpp
/// @brief Implementation of HealthBarSystem for rendering health bars
/// @namespace gme
///

#include "RTypeClientMulti/Systems/HealthBarSystem.hpp"
#include "ECS/Component.hpp"
#include "Utils/Logger.hpp"
#include <algorithm>

namespace gme
{
    HealthBarSystem::HealthBarSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}

    void HealthBarSystem::update(ecs::Registry &registry, float /*dt*/)
    {
        // Iterate through all entities with Player component
        auto playerEntities = registry.getAll<ecs::Player>();

        for (const auto &[entity, player] : playerEntities)
        {
            auto *transform = registry.getComponent<ecs::Transform>(entity);
            auto *health = registry.getComponent<ecs::Health>(entity);

            // Only render health bars for players with health
            if (transform && health)
            {
                // Don't show health bar if at full health (optional: remove this to always show)
                if (health->current < health->max)
                {
                    drawHealthBar(transform->x, transform->y, health->current, health->max);
                }
            }
        }
    }

    eng::Color HealthBarSystem::getHealthColor(float healthPercent) const
    {
        if (healthPercent > 0.75f)
        {
            return HEALTH_HIGH; // Green
        }
        else if (healthPercent > 0.50f)
        {
            return HEALTH_MEDIUM; // Yellow
        }
        else if (healthPercent > 0.25f)
        {
            return HEALTH_LOW; // Orange
        }
        else
        {
            return HEALTH_CRITICAL; // Red
        }
    }

    void HealthBarSystem::drawHealthBar(float x, float y, float currentHealth, float maxHealth)
    {
        if (maxHealth <= 0.0f)
            return;

        // Calculate health percentage
        float healthPercent = std::clamp(currentHealth / maxHealth, 0.0f, 1.0f);

        // Calculate bar position (centered above entity)
        float barX = x - (BAR_WIDTH / 2.0f);
        float barY = y + BAR_OFFSET_Y;

        // TODO: IRenderer doesn't support rectangle drawing yet
        // For now, we use circles to represent health
        // Draw health indicator using circle shapes
        std::string healthCircleName =
            "health_" + std::to_string(static_cast<int>(x)) + "_" + std::to_string(static_cast<int>(y));

        eng::Color healthColor = getHealthColor(healthPercent);
        float circleRadius = 3.0f;

        m_renderer->createCircleShape({.name = healthCircleName,
                                       .radius = circleRadius,
                                       .color = healthColor,
                                       .x = barX + (BAR_WIDTH * healthPercent),
                                       .y = barY});
        m_renderer->drawCircleShape(healthCircleName);

        (void)barX; // Suppress unused variable warning
        (void)barY;
        (void)healthPercent;
    }

} // namespace gme
