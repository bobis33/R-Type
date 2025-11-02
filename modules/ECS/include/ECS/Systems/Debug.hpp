///
/// @file Debug.hpp
/// @brief This file contains the debug system definition
/// @namespace ecs
///

#pragma once

#include <functional>
#include <ranges>
#include <string>

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
            using EntityFilter = std::function<bool(Registry &, Entity)>;

            explicit DebugSystem(const std::shared_ptr<eng::IRenderer> &renderer, bool &showDebug,
                                 EntityFilter filter = nullptr)
                : m_renderer(renderer), m_showDebug(showDebug), m_filter(filter)
            {
            }

            ~DebugSystem() override = default;

            DebugSystem(const DebugSystem &) = delete;
            DebugSystem &operator=(const DebugSystem &) = delete;
            DebugSystem(DebugSystem &&) = delete;
            DebugSystem &operator=(const DebugSystem &&) = delete;

            void update(Registry &registry, float /* dt */) override
            {
                const auto &circleShapes = registry.getAll<Hitbox>();
                for (const auto &pair : circleShapes)
                {
                    const auto key = pair.first;
                    if (!registry.hasComponent<Hitbox>(key) || !registry.hasComponent<Transform>(key))
                    {
                        continue;
                    }

                    if (m_filter && !m_filter(registry, key))
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
                    const std::string circleName = "hitbox_" + std::to_string(key);
                    try
                    {
                        m_renderer->setCircleShapePosition(circleName, hitboxX, hitboxY);
                        if (m_showDebug)
                        {
                            m_renderer->drawCircleShape(circleName);
                        }
                    }
                    catch (const std::runtime_error &)
                    {
                        continue;
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            bool &m_showDebug;
            EntityFilter m_filter;

    }; // class DebugSystem
} // namespace ecs
