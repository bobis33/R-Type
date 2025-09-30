///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include <cmath>
#include <iostream>
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/IRenderer.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

            void update(ecs::Registry &registry, float /* dt */) override
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

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, sprite] : registry.getAll<ecs::Texture>())
                {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    const auto *rect = registry.getComponent<ecs::Rect>(entity);

                    const float x = (transform != nullptr) ? transform->x : 0.F;
                    const float y = (transform != nullptr) ? transform->y : 0.F;
                    m_renderer.setSpriteTexture(sprite.id + std::to_string(entity), sprite.path);
                    m_renderer.setSpritePosition(sprite.id + std::to_string(entity), x, y);
                    
                    if (rect)
                    {
                        m_renderer.setSpriteFrame(sprite.id + std::to_string(entity), 
                                                 static_cast<int>(rect->pos_x), 
                                                 static_cast<int>(rect->pos_y), 
                                                 rect->size_x, 
                                                 rect->size_y);
                    }

                    m_renderer.drawSprite(sprite.id + std::to_string(entity));
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class SpriteSystem

    class AnimationSystem final : public eng::ASystem
    {
        public:
            explicit AnimationSystem(eng::IRenderer &/* renderer */) {}
            ~AnimationSystem() override = default;

            AnimationSystem(const AnimationSystem &) = delete;
            AnimationSystem &operator=(const AnimationSystem &) = delete;
            AnimationSystem(AnimationSystem &&) = delete;
            AnimationSystem &operator=(AnimationSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, animation] : registry.getAll<ecs::Animation>())
                {
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    
                    if (rect)
                    {
                        // Calculer la position du frame dans la spritesheet
                        int frame_x = (animation.current_frame % animation.frames_per_row) * animation.frame_width;
                        int frame_y = (animation.current_frame / animation.frames_per_row) * animation.frame_height;
                        if (rect->pos_x != static_cast<float>(frame_x) || rect->pos_y != static_cast<float>(frame_y))
                        {
                            rect->pos_x = static_cast<float>(frame_x);
                            rect->pos_y = static_cast<float>(frame_y);
                            rect->size_x = animation.frame_width;
                            rect->size_y = animation.frame_height;
                            
                        }
                    }
                }
            }

    }; // class AnimationSystem

    class PlayerDirectionSystem final : public eng::ASystem
    {
        public:
            explicit PlayerDirectionSystem() = default;
            ~PlayerDirectionSystem() override = default;

            PlayerDirectionSystem(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem &operator=(const PlayerDirectionSystem &) = delete;
            PlayerDirectionSystem(PlayerDirectionSystem &&) = delete;
            PlayerDirectionSystem &operator=(PlayerDirectionSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, animation] : registry.getAll<ecs::Animation>())
                {
                    const auto *player = registry.getComponent<ecs::Player>(entity);
                    const auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    
                    if (player && velocity)
                    {
                        int frame = 0;
                        float angle = std::atan2(velocity->y, velocity->x);
                        if (std::abs(velocity->x) < 0.1f && std::abs(velocity->y) < 0.1f)
                        {
                            return;
                        }
                        if (angle < 0) angle += 2.0f * static_cast<float>(M_PI);
                        if (angle >= 0 && angle < M_PI/4)
                            frame = 0; // Droite
                        else if (angle >= M_PI/4 && angle < 3*M_PI/4)
                            frame = 1; // Haut
                        else if (angle >= 3*M_PI/4 && angle < 5*M_PI/4)
                            frame = 2; // Gauche
                        else if (angle >= 5*M_PI/4 && angle < 7*M_PI/4)
                            frame = 3; // Bas
                        else
                            frame = 4; // Droite (retour)
                        animation.current_frame = frame;
                    }
                }
            }
    }; // class PlayerDirectionSystem

    class PixelSystem final : public eng::ASystem
    {
        public:
            explicit PixelSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~PixelSystem() override = default;

            explicit PixelSystem(const SpriteSystem &) = delete;
            PixelSystem &operator=(const SpriteSystem &) = delete;
            explicit PixelSystem(SpriteSystem &&) = delete;
            PixelSystem &operator=(SpriteSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                for (auto &[entity, pixel] : registry.getAll<ecs::Pixel>())
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