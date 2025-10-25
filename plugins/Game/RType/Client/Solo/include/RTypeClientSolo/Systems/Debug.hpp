///
/// @file Debug.hpp
/// @brief This file contains the debug system definitions
/// @namespace gme
///

#pragma once

#include <ranges>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    class DebugSystem final : public eng::ASystem
    {
        public:
            explicit DebugSystem(const std::shared_ptr<eng::IRenderer> &renderer, bool &showDebug)
                : m_renderer(renderer), m_showDebug(showDebug)
            {
            }

            ~DebugSystem() override = default;

            DebugSystem(const DebugSystem &) = delete;
            DebugSystem &operator=(const DebugSystem &) = delete;
            DebugSystem(DebugSystem &&) = delete;
            DebugSystem &operator=(DebugSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {

                const auto &circleShapes = registry.getAll<ecs::Hitbox>();
                const auto &transforms = registry.getAll<ecs::Transform>();
                for (const auto &key : circleShapes | std::views::keys)
                {
                    if (!registry.hasComponent<ecs::Hitbox>(key) || !registry.hasComponent<ecs::Transform>(key))
                    {
                        continue;
                    }

                    const auto *transform = registry.getComponent<ecs::Transform>(key);
                    const auto *hitbox = registry.getComponent<ecs::Hitbox>(key);

                    if ((transform == nullptr) || (hitbox == nullptr))
                    {
                        continue;
                    }
                    float hitboxX = transform->x + hitbox->offsetX - hitbox->radius;
                    float hitboxY = transform->y + hitbox->offsetY - hitbox->radius;
                    m_renderer->setCircleShapePosition("hitbox_" + std::to_string(key), hitboxX, hitboxY);
                    if (m_showDebug)
                    {
                        m_renderer->drawCircleShape("hitbox_" + std::to_string(key));
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            bool &m_showDebug;

    }; // class AnimationSystem

} // namespace gme