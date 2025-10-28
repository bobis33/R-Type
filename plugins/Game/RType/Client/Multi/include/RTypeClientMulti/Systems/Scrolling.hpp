#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "RTypeShared/GameConfig.hpp"

namespace gme
{
    class ScrollingSystem final : public ecs::ASystem
    {
        public:
            explicit ScrollingSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~ScrollingSystem() override = default;

            ScrollingSystem(const ScrollingSystem &) = delete;
            ScrollingSystem &operator=(const ScrollingSystem &) = delete;
            ScrollingSystem(ScrollingSystem &&) = delete;
            ScrollingSystem &operator=(const ScrollingSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                const auto [width, height] = m_renderer->getWindowSize();

                for (auto &[entity, scrolling] : registry.getAll<ecs::Scrolling>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);
                    const bool isFloor = registry.hasComponent<ecs::Floor>(entity);
                    const bool isCeiling = registry.hasComponent<ecs::Ceiling>(entity);

                    if (transform == nullptr)
                    {
                        continue;
                    }

                    if (scrolling.fit_width && (scale != nullptr))
                    {
                        const auto targetWidth = static_cast<float>(width);
                        const float scaleX = targetWidth / std::max(1.0f, scrolling.original_width);
                        scale->x = scaleX;
                        scale->y = scaleX;
                    }

                    const float scaledHeight = ((scale != nullptr) ? scale->y : 1.0f) * scrolling.original_height;
                    if (isCeiling)
                    {
                        transform->y = GameConfig::Stage::CEILING_OFFSET_Y;
                    }
                    else if (isFloor)
                    {
                        transform->y = static_cast<float>(height) - scaledHeight - GameConfig::Stage::FLOOR_OFFSET_Y;
                    }

                    transform->x += scrolling.speed_x * dt;

                    if (const float scaledWidth = ((scale != nullptr) ? scale->x : 1.0f) * scrolling.original_width;
                        transform->x + scaledWidth < 0.0f)
                    {
                        transform->x += scaledWidth + static_cast<float>(width);
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    };
} // namespace gme

