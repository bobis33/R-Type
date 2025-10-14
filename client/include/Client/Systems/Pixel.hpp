///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    class PixelSystem final : public eng::ASystem
    {
        public:
            explicit PixelSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~PixelSystem() override = default;

            explicit PixelSystem(const PixelSystem &) = delete;
            PixelSystem &operator=(const PixelSystem &) = delete;
            explicit PixelSystem(PixelSystem &&) = delete;
            PixelSystem &operator=(PixelSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, pixel] : registry.getAll<ecs::Pixel>())
                {
                    const auto *color = registry.getComponent<ecs::Color>(entity);
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    m_renderer->drawPoint(transform->x, transform->y,
                                         {.r = color->r, .g = color->g, .b = color->b, .a = color->a});
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class PixelSystem

} // namespace cli