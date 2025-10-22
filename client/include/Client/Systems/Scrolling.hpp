///
/// @file Scrolling.hpp
/// @brief Horizontal scrolling system for large floor/ceiling sprites
/// @namespace cli
///

#pragma once

#include "Client/GameConfig.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class ScrollingSystem final : public eng::ASystem
    {
        public:
            explicit ScrollingSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~ScrollingSystem() override = default;

            ScrollingSystem(const ScrollingSystem &) = delete;
            ScrollingSystem &operator=(const ScrollingSystem &) = delete;
            ScrollingSystem(ScrollingSystem &&) = delete;
            ScrollingSystem &operator=(ScrollingSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                const auto window = m_renderer->getWindowSize();

                for (auto &[entity, scrolling] : registry.getAll<ecs::Scrolling>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);
                    const bool isFloor = registry.hasComponent<ecs::Floor>(entity);
                    const bool isCeiling = registry.hasComponent<ecs::Ceiling>(entity);

                    if (!transform)
                        continue;

                    // Fit width if requested (uniform scale to preserve aspect ratio)
                    if (scrolling.fit_width && scale)
                    {
                        const float targetWidth = static_cast<float>(window.width);
                        const float scaleX = targetWidth / std::max(1.0f, scrolling.original_width);
                        scale->x = scaleX;
                        scale->y = scaleX;
                    }

                    // Align vertically if tagged
                    const float scaledHeight = (scale ? scale->y : 1.0f) * scrolling.original_height;
                    if (isCeiling)
                    {
                        transform->y = GameConfig::Stage::CEILING_OFFSET_Y;
                    }
                    else if (isFloor)
                    {
                        transform->y =
                            static_cast<float>(window.height) - scaledHeight - GameConfig::Stage::FLOOR_OFFSET_Y;
                    }

                    // Move horizontally
                    transform->x += scrolling.speed_x * dt;

                    // Wrap when completely out of view on the left
                    const float scaledWidth = (scale ? scale->x : 1.0f) * scrolling.original_width;
                    if (transform->x + scaledWidth < 0.0f)
                    {
                        transform->x += scaledWidth + static_cast<float>(window.width);
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class ScrollingSystem

} // namespace cli
