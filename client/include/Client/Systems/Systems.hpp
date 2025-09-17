///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

#include "Engine/Systems.hpp"

namespace cli
{

    ///
    /// @class TextSyStem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class TextSyStem final : public eng::ISystem
    {
        public:
            explicit TextSyStem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~TextSyStem() override = default;

            TextSyStem(const TextSyStem &) = delete;
            TextSyStem &operator=(const TextSyStem &) = delete;
            TextSyStem(TextSyStem &&) = delete;
            TextSyStem &operator=(TextSyStem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {

                for (auto &[entity, text] : registry.getAll<ecs::Text>()) {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    const auto *color     = registry.getComponent<ecs::Color>(entity);

                    int x = transform ? static_cast<int>(transform->x) : 0;
                    int y = transform ? static_cast<int>(transform->y) : 0;

                    std::uint8_t r = color ? static_cast<std::uint8_t>(color->r) : 255;
                    std::uint8_t g = color ? static_cast<std::uint8_t>(color->g) : 255;
                    std::uint8_t b = color ? static_cast<std::uint8_t>(color->b) : 255;
                    std::uint8_t a = color ? static_cast<std::uint8_t>(color->a) : 255;

                    m_renderer.setTextContent(text.id, text.content);
                    m_renderer.setTextPosition(text.id, x, y);
                    m_renderer.setTextColor(text.id, {r, g, b, a});
                    m_renderer.drawText(text.id);
                }
            }

            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) override { m_isEnable = enable; }

        private:
            eng::IRenderer &m_renderer;
            bool m_isEnable = true;
    }; // class TextRenderSystem

    ///
    /// @class FontSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class FontSystem final : public eng::ISystem
    {
        public:
            explicit FontSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~FontSystem() override = default;

            FontSystem(const FontSystem &) = delete;
            FontSystem &operator=(const FontSystem &) = delete;
            FontSystem(FontSystem &&) = delete;
            FontSystem &operator=(FontSystem &&) = delete;

            void update(ecs::Registry &registry, const float dt) override {}
            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) override { m_isEnable = enable; }

        private:
            eng::IRenderer &m_renderer;
            bool m_isEnable = true;
    }; // class FontSystem

    ///
    /// @class AudioSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class AudioSystem final : public eng::ISystem
    {
        public:
            explicit AudioSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~AudioSystem() override = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override {}
            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) override { m_isEnable = enable; }

        private:
            eng::IRenderer &m_renderer;
            bool m_isEnable = true;
    }; // class AudioSystem

} // namespace eng