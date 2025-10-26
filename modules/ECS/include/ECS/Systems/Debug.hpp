///
/// @file Debug.hpp
/// @brief This file contains the debug system definition
/// @namespace ecs
///

#pragma once

#include <ranges>

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace ecs
{

    ///
    /// @class DebugSystem
    /// @brief Class for debug system
    /// @namespace ecs
    ///
    class DebugSystem final : public ASystem
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

            void update(Registry &registry, float dt) override
            {

                const auto &circleShapes = registry.getAll<Hitbox>();
                const auto &transforms = registry.getAll<Transform>();
                for (const auto &key : circleShapes | std::views::keys)
                {
                    if (!registry.hasComponent<Hitbox>(key) || !registry.hasComponent<Transform>(key))
                    {
                        continue;
                    }

                    const auto *transform = registry.getComponent<Transform>(key);
                    const auto *hitbox = registry.getComponent<Hitbox>(key);

                    if ((transform == nullptr) || (hitbox == nullptr))
                    {
                        continue;
                    }
                    const float hitboxX = transform->x + hitbox->offsetX - hitbox->radius;
                    const float hitboxY = transform->y + hitbox->offsetY - hitbox->radius;
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

    }; // class DebugSystem
} // namespace ecs