///
/// @file FlappyDebugSystem.hpp
/// @brief This file contains the FlappyBird debug system definition
/// @namespace gme
///

#pragma once

#include <ranges>

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    ///
    /// @class FlappyDebugSystem
    /// @brief Class for FlappyBird debug system that excludes pipes from circle hitbox display
    /// @namespace gme
    ///
    class FlappyDebugSystem final : public ecs::ASystem
    {
        public:
            explicit FlappyDebugSystem(const std::shared_ptr<eng::IRenderer> &renderer, bool &showDebug)
                : m_renderer(renderer), m_showDebug(showDebug)
            {
            }

            ~FlappyDebugSystem() override = default;

            FlappyDebugSystem(const FlappyDebugSystem &) = delete;
            FlappyDebugSystem &operator=(const FlappyDebugSystem &) = delete;
            FlappyDebugSystem(FlappyDebugSystem &&) = delete;
            FlappyDebugSystem &operator=(const FlappyDebugSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                const auto &circleShapes = registry.getAll<ecs::Hitbox>();
                for (const auto &[key, hitboxComponent] : circleShapes)
                {
                    if (!registry.hasComponent<ecs::Hitbox>(key) || !registry.hasComponent<ecs::Transform>(key))
                    {
                        continue;
                    }

                    if (registry.hasComponent<ecs::Texture>(key))
                    {
                        const auto *texture = registry.getComponent<ecs::Texture>(key);
                        if (texture && (texture->id.find("pipe") != std::string::npos))
                        {
                            continue;
                        }
                    }

                    const auto *transform = registry.getComponent<ecs::Transform>(key);
                    const auto *hitbox = registry.getComponent<ecs::Hitbox>(key);

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
    }; // class FlappyDebugSystem
} // namespace gme

