///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/IRenderer.hpp"

#include "Engine/Systems.hpp"

namespace cli
{

    ///
    /// @class TextSyStem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class TextSyStem final : public eng::ASystem
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
    }; // class TextRenderSystem

    ///
    /// @class AudioSystem
    /// @brief Class for managing entities and their components
    /// @namespace ecs
    ///
    class AudioSystem final : public eng::ASystem
    {
        public:
            explicit AudioSystem(eng::IAudio &audio) : m_audio(audio) {}
            ~AudioSystem() override = default;

            AudioSystem(const AudioSystem &) = delete;
            AudioSystem &operator=(const AudioSystem &) = delete;
            AudioSystem(AudioSystem &&) = delete;
            AudioSystem &operator=(AudioSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, audio] : registry.getAll<ecs::Audio>())
                {
                    m_audio.setVolume(audio.id + std::to_string(entity), audio.volume);
                    m_audio.setLoop(audio.id + std::to_string(entity), audio.loop);
                    if (audio.play && m_audio.isPlaying(audio.id + std::to_string(entity)) != eng::Status::Playing)
                    {
                        m_audio.playAudio(audio.id + std::to_string(entity));
                    }
                    else if (!audio.play &&
                             m_audio.isPlaying(audio.id + std::to_string(entity)) != eng::Status::Stopped)
                    {
                        m_audio.stopAudio(audio.id + std::to_string(entity));
                    }
                }
            }

        private:
            eng::IAudio &m_audio;
    }; // class AudioSystem

    class SpriteSystem final : public eng::ASystem
    {
        public:
            explicit SpriteSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~SpriteSystem() override = default;

            SpriteSystem(const SpriteSystem &) = delete;
            SpriteSystem &operator=(const SpriteSystem &) = delete;
            SpriteSystem(SpriteSystem &&) = delete;
            SpriteSystem &operator=(SpriteSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, sprite] : registry.getAll<ecs::Texture>())
                {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    const auto *color = registry.getComponent<ecs::Color>(entity);
                    const auto *velocity = registry.getComponent<ecs::Velocity>(entity);

                    const float x = (transform != nullptr) ? transform->x : 0.F;
                    const float y = (transform != nullptr) ? transform->y : 0.F;

                    // std::uint8_t r = color ? static_cast<std::uint8_t>(color->r) : 255;
                    // std::uint8_t g = color ? static_cast<std::uint8_t>(color->g) : 255;
                    // std::uint8_t b = color ? static_cast<std::uint8_t>(color->b) : 255;
                    // std::uint8_t a = color ? static_cast<std::uint8_t>(color->a) : 255;

                    // int xv = velocity ? static_cast<int>(velocity->x) : 0;
                    // int yv = velocity ? static_cast<int>(velocity->y) : 0;
                    // x *= xv;
                    // y *= yv;
                    m_renderer.setSpriteTexture(sprite.id + std::to_string(entity), sprite.path);
                    m_renderer.setSpritePosition(sprite.id + std::to_string(entity), x, y);
                    // m_renderer.setSpriteColor(sprite.id, {r, g, b, a});

                    m_renderer.drawSprite(sprite.id + std::to_string(entity));
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class SpriteSystem

    class PixelSystem final : public eng::ASystem
    {
        public:
            explicit PixelSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~PixelSystem() override = default;

            explicit PixelSystem(const SpriteSystem &) = delete;
            PixelSystem &operator=(const SpriteSystem &) = delete;
            explicit PixelSystem(SpriteSystem &&) = delete;
            PixelSystem &operator=(SpriteSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (const auto &entity : registry.getAll<ecs::Pixel>() | std::views::keys)
                {
                    const auto *color = registry.getComponent<ecs::Color>(entity);
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    m_renderer.drawPoint(transform->x, transform->y,
                                         {.r = color->r, .g = color->g, .b = color->b, .a = color->a});
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class PixelSystem

} // namespace cli