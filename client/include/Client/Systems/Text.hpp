///
/// @file Systems.hpp
/// @brief This file contains the TextSystem definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{

    ///
    /// @class TextSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class TextSystem final : public eng::ASystem
    {
        public:
            explicit TextSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~TextSystem() override = default;

            TextSystem(const TextSystem &) = delete;
            TextSystem &operator=(const TextSystem &) = delete;
            TextSystem(TextSystem &&) = delete;
            TextSystem &operator=(TextSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {

                for (auto &[entity, text] : registry.getAll<ecs::Text>())
                {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    const auto *color = registry.getComponent<ecs::Color>(entity);

                    const float x = (transform != nullptr) ? transform->x : 0.F;
                    const float y = (transform != nullptr) ? transform->y : 0.F;

                    const std::uint8_t r = color ? color->r : 255u;
                    const std::uint8_t g = color ? color->g : 255u;
                    const std::uint8_t b = color ? color->b : 255u;
                    const std::uint8_t a = color ? color->a : 255u;

                    m_renderer.setTextContent(text.id, text.content);
                    m_renderer.setTextPosition(text.id, x, y);
                    m_renderer.setTextColor(text.id, {.r = r, .g = g, .b = b, .a = a});
                    m_renderer.drawText(text.id);
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class TextSystem

} // namespace cli