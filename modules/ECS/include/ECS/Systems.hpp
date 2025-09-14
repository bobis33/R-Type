///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace ecs
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"

namespace ecs
{

    ///
    /// @class TextSyStem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class TextSyStem
    {
        public:
            TextSyStem() = default;
            ~TextSyStem() = default;

            TextSyStem(const TextSyStem &) = delete;
            TextSyStem &operator=(const TextSyStem &) = delete;
            TextSyStem(TextSyStem &&) = delete;
            TextSyStem &operator=(TextSyStem &&) = delete;

            using DrawCallback = std::function<void(const Text &, const Transform &, const Color &)>;

            void setDrawCallback(DrawCallback cb) { m_drawCallback = std::move(cb); }

            void update(Registry &registry) const
            {
                for (auto &[entity, text] : registry.getAll<Text>())
                {
                    const auto *transform = registry.getComponent<Transform>(entity);
                    const auto *color = registry.getComponent<Color>(entity);
                    if (!transform)
                        continue;

                    if (m_drawCallback)
                        m_drawCallback(text, *transform, *color);
                }
            }

        private:
            DrawCallback m_drawCallback;
    }; // class TextRenderSystem

    ///
    /// @class FontSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class FontSystem
    {
        public:
            FontSystem() = default;
            ~FontSystem() = default;

            FontSystem(const FontSystem &) = delete;
            FontSystem &operator=(const FontSystem &) = delete;
            FontSystem(FontSystem &&) = delete;
            FontSystem &operator=(FontSystem &&) = delete;

            void update(Registry &registry) {}

        private:
    }; // class FontSystem

    ///
    /// @class AudioSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class AudioSystem
    {
        public:
            AudioSystem() = default;
            ~AudioSystem() = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(Registry &registry, float dt) {}

        private:
    }; // class AudioSystem

} // namespace ecs