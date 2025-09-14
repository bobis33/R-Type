///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace eng
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace eng
{

    class ISystem
    {
        public:
            virtual ~ISystem() = default;
            virtual void update(ecs::Registry &registry, float dt) = 0;
            virtual bool isEnable() = 0;
    };

    ///
    /// @class TextSyStem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class TextSyStem final : public ISystem
    {
        public:
            explicit TextSyStem(IRenderer &renderer) : m_renderer(renderer) {}
            ~TextSyStem() override = default;

            TextSyStem(const TextSyStem &) = delete;
            TextSyStem &operator=(const TextSyStem &) = delete;
            TextSyStem(TextSyStem &&) = delete;
            TextSyStem &operator=(TextSyStem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, text] : registry.getAll<ecs::Text>())
                {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    const auto *color = registry.getComponent<ecs::Color>(entity);
                    if (!transform || !color)
                        continue;
                    m_renderer.setTextContent(text.id, text.content);
                    m_renderer.setTextPosition(text.id, static_cast<int>(transform->x), static_cast<int>(transform->y));
                    m_renderer.setTextColor(text.id, {.r = static_cast<std::uint8_t>(color->r),
                                                      .g = static_cast<std::uint8_t>(color->g),
                                                      .b = static_cast<std::uint8_t>(color->b),
                                                      .a = static_cast<std::uint8_t>(color->a)});
                    m_renderer.drawText(text.id);
                }
            }

            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) { m_isEnable = enable; }

        private:
            IRenderer &m_renderer;
            bool m_isEnable = true;
    }; // class TextRenderSystem

    ///
    /// @class FontSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class FontSystem final : public ISystem
    {
        public:
            explicit FontSystem(IRenderer &renderer) : m_renderer(renderer) {}
            ~FontSystem() override = default;

            FontSystem(const FontSystem &) = delete;
            FontSystem &operator=(const FontSystem &) = delete;
            FontSystem(FontSystem &&) = delete;
            FontSystem &operator=(FontSystem &&) = delete;

            void update(ecs::Registry &registry, const float dt) override {}
            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) { m_isEnable = enable; }

        private:
            IRenderer &m_renderer;
            bool m_isEnable = true;
    }; // class FontSystem

    ///
    /// @class AudioSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class AudioSystem final : public ISystem
    {
        public:
            explicit AudioSystem(IRenderer &renderer) : m_renderer(renderer) {}
            ~AudioSystem() override = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override {}
            bool isEnable() override { return m_isEnable; }
            void setEnable(const bool enable) { m_isEnable = enable; }

        private:
            IRenderer &m_renderer;
            bool m_isEnable = true;
    }; // class AudioSystem

} // namespace eng