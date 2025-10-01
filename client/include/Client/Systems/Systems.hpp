///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace cli
///

#pragma once

#include <cmath>
#include <vector>
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "../GameConfig.hpp"
// #include "../SpriteRect.hpp" // Temporairement désactivé
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

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, animation] : registry.getAll<ecs::Animation>())
                {
                    animation.current_time += dt;
                    
                    if (animation.current_time >= animation.frame_duration)
                    {
                        animation.current_time = 0.0f;
                        animation.current_frame = (animation.current_frame + 1) % animation.total_frames;
                    }
                    
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
                for (auto &[entity, player] : registry.getAll<ecs::Player>())
                {
                    const auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    
                    if (velocity && rect)
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
                        int frame_width = static_cast<int>(GameConfig::Player::SPRITE_WIDTH);
                        int frame_height = static_cast<int>(GameConfig::Player::SPRITE_HEIGHT);
                        int frames_per_row = GameConfig::Player::FRAMES_PER_ROW;
                        int frame_x = (frame % frames_per_row) * frame_width;
                        int frame_y = (frame / frames_per_row) * frame_height;
                        
                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                        rect->size_x = frame_width;
                        rect->size_y = frame_height;
                    }
                }
            }
    }; // class PlayerDirectionSystem

    class ProjectileSystem final : public eng::ASystem
    {
        public:
            explicit ProjectileSystem(eng::IRenderer &/* renderer */) {}
            ~ProjectileSystem() override = default;

            ProjectileSystem(const ProjectileSystem &) = delete;
            ProjectileSystem &operator=(const ProjectileSystem &) = delete;
            ProjectileSystem(ProjectileSystem &&) = delete;
            ProjectileSystem &operator=(ProjectileSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> entitiesToRemove;
                
                for (auto &[entity, projectile] : registry.getAll<ecs::Projectile>())
                {
                    projectile.current_lifetime += dt;
                    if (projectile.current_lifetime >= projectile.lifetime)
                    {
                        entitiesToRemove.push_back(entity);
                        continue;
                    }
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    
                    if (transform && velocity)
                    {
                        transform->x += velocity->x * dt;
                        transform->y += velocity->y * dt;
                    }
                }
                
                for (const auto &entity : entitiesToRemove)
                {
                    if (registry.hasComponent<ecs::Projectile>(entity))
                        registry.removeComponent<ecs::Projectile>(entity);
                    if (registry.hasComponent<ecs::Transform>(entity))
                        registry.removeComponent<ecs::Transform>(entity);
                    if (registry.hasComponent<ecs::Velocity>(entity))
                        registry.removeComponent<ecs::Velocity>(entity);
                    if (registry.hasComponent<ecs::Rect>(entity))
                        registry.removeComponent<ecs::Rect>(entity);
                    if (registry.hasComponent<ecs::Scale>(entity))
                        registry.removeComponent<ecs::Scale>(entity);
                    if (registry.hasComponent<ecs::Texture>(entity))
                        registry.removeComponent<ecs::Texture>(entity);
                    if (registry.hasComponent<ecs::Animation>(entity))
                        registry.removeComponent<ecs::Animation>(entity);
                }
            }

    }; // class ProjectileSystem

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

    class BeamSystem final : public eng::ASystem
    {
        public:
            explicit BeamSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~BeamSystem() override = default;

            BeamSystem(const BeamSystem &) = delete;
            BeamSystem &operator=(const BeamSystem &) = delete;
            BeamSystem(BeamSystem &&) = delete;
            BeamSystem &operator=(BeamSystem &&) = delete;

            void update(ecs::Registry &registry, float /* dt */) override
            {
                // Chercher seulement le joueur avec BeamCharge
                for (auto &[entity, beamCharge] : registry.getAll<ecs::BeamCharge>())
                {
                    const auto *player = registry.getComponent<ecs::Player>(entity);
                    if (!player) continue; // Seulement pour le joueur
                    
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    if (!transform) continue;

                    // Position de la barre au-dessus du joueur
                    float barX = transform->x + GameConfig::Player::SPRITE_WIDTH / 2.0f - GameConfig::Beam::BAR_WIDTH / 2.0f;
                    float barY = transform->y - GameConfig::Beam::BAR_HEIGHT - 10.0f; // 10 pixels au-dessus

                    // Dessiner la barre de fond avec des points (plus petite et plus fine)
                    for (int x = 0; x < static_cast<int>(GameConfig::Beam::BAR_WIDTH); x += 3)
                    {
                        for (int y = 0; y < static_cast<int>(GameConfig::Beam::BAR_HEIGHT); y += 3)
                        {
                            m_renderer.drawPoint(barX + x, barY + y, 
                                               {.r = 30, .g = 30, .b = 30, .a = 200});
                        }
                    }

                    // Dessiner la barre de chargement
                    float chargeRatio = beamCharge.current_charge / beamCharge.max_charge;
                    float chargeWidth = GameConfig::Beam::BAR_WIDTH * chargeRatio;
                    
                    // Dessiner le seuil de 50% (ligne verticale)
                    float thresholdX = barX + GameConfig::Beam::BAR_WIDTH * 0.5f;
                    for (int y = 0; y < static_cast<int>(GameConfig::Beam::BAR_HEIGHT); y += 2)
                    {
                        m_renderer.drawPoint(thresholdX, barY + y, {.r = 255, .g = 255, .b = 255, .a = 150}); // Ligne blanche semi-transparente
                    }
                    
                    if (chargeWidth > 0)
                    {
                        for (int x = 0; x < static_cast<int>(chargeWidth); x += 3)
                        {
                            for (int y = 0; y < static_cast<int>(GameConfig::Beam::BAR_HEIGHT); y += 3)
                            {
                                // Couleur qui change selon le niveau de charge
                                eng::Color chargeColor;
                                if (chargeRatio < 0.5f) {
                                    chargeColor = {.r = 255, .g = 100, .b = 0, .a = 255}; // Orange (en dessous du seuil)
                                } else if (chargeRatio < 0.8f) {
                                    chargeColor = {.r = 255, .g = 200, .b = 0, .a = 255}; // Jaune
                                } else {
                                    chargeColor = {.r = 0, .g = 255, .b = 0, .a = 255}; // Vert
                                }
                                
                                m_renderer.drawPoint(barX + x, barY + y, chargeColor);
                            }
                        }
                    }
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class BeamSystem

    class LoadingAnimationSystem final : public eng::ASystem
    {
        public:
            explicit LoadingAnimationSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~LoadingAnimationSystem() override = default;

            LoadingAnimationSystem(const LoadingAnimationSystem &) = delete;
            LoadingAnimationSystem &operator=(const LoadingAnimationSystem &) = delete;
            LoadingAnimationSystem(LoadingAnimationSystem &&) = delete;
            LoadingAnimationSystem &operator=(LoadingAnimationSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override
            {
                for (auto &[entity, animation] : registry.getAll<ecs::LoadingAnimation>())
                {
                    const auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    const auto *texture = registry.getComponent<ecs::Texture>(entity);
                    
                    if (!transform || !rect || !texture) continue;

                    // Mettre à jour l'animation
                    animation.current_time += dt;
                    if (animation.current_time >= animation.frame_duration)
                    {
                        animation.current_time = 0.0f;
                        animation.current_frame = (animation.current_frame + 1) % animation.total_frames;
                        
                        // Mettre à jour le rectangle de texture
                        int frame_x = (animation.current_frame % animation.frames_per_row) * static_cast<int>(animation.frame_width);
                        int frame_y = (animation.current_frame / animation.frames_per_row) * static_cast<int>(animation.frame_height);
                        
                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    // Dessiner l'animation
                    m_renderer.setSpriteTexture(texture->id + std::to_string(entity), texture->path);
                    m_renderer.setSpritePosition(texture->id + std::to_string(entity), transform->x, transform->y);
                    m_renderer.setSpriteFrame(texture->id + std::to_string(entity), 
                                            static_cast<int>(rect->pos_x), 
                                            static_cast<int>(rect->pos_y), 
                                            rect->size_x, 
                                            rect->size_y);
                    m_renderer.drawSprite(texture->id + std::to_string(entity));
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class LoadingAnimationSystem

    class EnemySystem final : public eng::ISystem
    {
        public:
            explicit EnemySystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~EnemySystem() override = default;

            EnemySystem(const EnemySystem &) = delete;
            EnemySystem &operator=(const EnemySystem &) = delete;
            EnemySystem(EnemySystem &&) = delete;
            EnemySystem &operator=(EnemySystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> enemiesToRemove;
                
                // Mettre à jour les ennemis existants
                for (auto &[entity, enemy] : registry.getAll<ecs::Enemy>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    auto *texture = registry.getComponent<ecs::Texture>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);
                    auto *animation = registry.getComponent<ecs::Animation>(entity);
                    
                    if (!transform || !velocity || !rect || !texture || !scale) continue;

                    // Mettre à jour la position
                    transform->x += velocity->x * dt;
                    transform->y += velocity->y * dt;

                // Animation simple
                if (animation)
                {
                    animation->current_time += dt;
                    if (animation->current_time >= animation->frame_duration)
                    {
                        animation->current_time = 0.0f;
                        animation->current_frame = (animation->current_frame + 1) % animation->total_frames;
                        
                        const int frame_x = animation->current_frame * static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH);
                        const int frame_y = 0;
                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }
                }

                    // Dessiner l'ennemi
                    m_renderer.createSprite(texture->id + std::to_string(entity), texture->path, 
                                          std::round(transform->x), std::round(transform->y), scale->x, scale->y,
                                          static_cast<int>(rect->pos_x), static_cast<int>(rect->pos_y),
                                          static_cast<int>(rect->size_x), static_cast<int>(rect->size_y));
                    m_renderer.drawSprite(texture->id + std::to_string(entity));

                    if (transform->x < GameConfig::Screen::REMOVE_X || 
                        transform->y < GameConfig::Screen::REMOVE_MIN_Y || transform->y > GameConfig::Screen::REMOVE_MAX_Y)
                    {
                        enemiesToRemove.push_back(entity);
                    }
                }
                
                for (ecs::Entity entity : enemiesToRemove)
                {
                    if (registry.hasComponent<ecs::Enemy>(entity))
                        registry.removeComponent<ecs::Enemy>(entity);
                    if (registry.hasComponent<ecs::Transform>(entity))
                        registry.removeComponent<ecs::Transform>(entity);
                    if (registry.hasComponent<ecs::Velocity>(entity))
                        registry.removeComponent<ecs::Velocity>(entity);
                    if (registry.hasComponent<ecs::Rect>(entity))
                        registry.removeComponent<ecs::Rect>(entity);
                    if (registry.hasComponent<ecs::Texture>(entity))
                        registry.removeComponent<ecs::Texture>(entity);
                    if (registry.hasComponent<ecs::Scale>(entity))
                        registry.removeComponent<ecs::Scale>(entity);
                    if (registry.hasComponent<ecs::Animation>(entity))
                        registry.removeComponent<ecs::Animation>(entity);
                    if (registry.hasComponent<ecs::Hitbox>(entity))
                        registry.removeComponent<ecs::Hitbox>(entity);
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class EnemySystem

    class AsteroidSystem final : public eng::ISystem
    {
        public:
            explicit AsteroidSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~AsteroidSystem() override = default;

            AsteroidSystem(const AsteroidSystem &) = delete;
            AsteroidSystem &operator=(const AsteroidSystem &) = delete;
            AsteroidSystem(AsteroidSystem &&) = delete;
            AsteroidSystem &operator=(AsteroidSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> asteroidsToRemove;
                
                for (auto &[entity, asteroid] : registry.getAll<ecs::Asteroid>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *velocity = registry.getComponent<ecs::Velocity>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    auto *texture = registry.getComponent<ecs::Texture>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);
                    auto *animation = registry.getComponent<ecs::Animation>(entity);
                    
                    if (!transform || !velocity || !rect || !texture || !scale || !animation) continue;

                    animation->current_time += dt;
                    if (animation->current_time >= animation->frame_duration)
                    {
                        animation->current_time = 0.0f;
                        animation->current_frame = (animation->current_frame + 1) % animation->total_frames;
                        
                        const int frame_x = animation->current_frame * static_cast<int>(GameConfig::Asteroid::Small::SPRITE_WIDTH);
                        const int frame_y = 0;
                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    transform->x += velocity->x * dt;
                    transform->y += velocity->y * dt;
                    transform->rotation += asteroid.rotation_speed * dt;

                    m_renderer.createSprite(texture->id + std::to_string(entity), texture->path, 
                                          std::round(transform->x), std::round(transform->y), scale->x, scale->y,
                                          static_cast<int>(rect->pos_x), static_cast<int>(rect->pos_y),
                                          static_cast<int>(rect->size_x), static_cast<int>(rect->size_y));
                    m_renderer.drawSprite(texture->id + std::to_string(entity));

                    if (transform->x < GameConfig::Screen::REMOVE_X || 
                        transform->y < GameConfig::Screen::REMOVE_MIN_Y || transform->y > GameConfig::Screen::REMOVE_MAX_Y)
                    {
                        asteroidsToRemove.push_back(entity);
                    }
                }
                
                for (ecs::Entity entity : asteroidsToRemove)
                {
                    if (registry.hasComponent<ecs::Asteroid>(entity))
                        registry.removeComponent<ecs::Asteroid>(entity);
                    if (registry.hasComponent<ecs::Transform>(entity))
                        registry.removeComponent<ecs::Transform>(entity);
                    if (registry.hasComponent<ecs::Velocity>(entity))
                        registry.removeComponent<ecs::Velocity>(entity);
                    if (registry.hasComponent<ecs::Rect>(entity))
                        registry.removeComponent<ecs::Rect>(entity);
                    if (registry.hasComponent<ecs::Texture>(entity))
                        registry.removeComponent<ecs::Texture>(entity);
                    if (registry.hasComponent<ecs::Scale>(entity))
                        registry.removeComponent<ecs::Scale>(entity);
                    if (registry.hasComponent<ecs::Hitbox>(entity))
                        registry.removeComponent<ecs::Hitbox>(entity);
                }
            }

        private:
            eng::IRenderer &m_renderer;
    }; // class AsteroidSystem

    class SpawnSystem final : public eng::ISystem
    {
        public:
            explicit SpawnSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~SpawnSystem() override = default;

            SpawnSystem(const SpawnSystem &) = delete;
            SpawnSystem &operator=(const SpawnSystem &) = delete;
            SpawnSystem(SpawnSystem &&) = delete;
            SpawnSystem &operator=(SpawnSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                m_enemySpawnTimer += dt;
                m_asteroidSpawnTimer += dt;
                m_waveTimer += dt;

                if (m_enemySpawnTimer >= GameConfig::Enemy::Easy::SPAWN_RATE)
                {
                    spawnEnemy(registry);
                    m_enemySpawnTimer = 0.0f;
                }

                if (m_waveTimer >= 10.0f)
                {
                    spawnWave(registry);
                    m_waveTimer = 0.0f;
                }

                if (m_asteroidSpawnTimer >= GameConfig::Asteroid::Small::SPAWN_RATE)
                {
                    spawnAsteroid(registry, ecs::Asteroid::SMALL);
                    m_asteroidSpawnTimer = 0.0f;
                }
            }

        private:
            eng::IRenderer &m_renderer;
            float m_enemySpawnTimer = 0.0f;
            float m_asteroidSpawnTimer = 0.0f;
            float m_waveTimer = 0.0f;

            void spawnEnemy(ecs::Registry &registry)
            {
                float x = GameConfig::Screen::SPAWN_X;
                float y = static_cast<float>(GameConfig::Screen::MIN_Y + (std::rand() % static_cast<int>(GameConfig::Screen::MAX_Y - GameConfig::Screen::MIN_Y)));
                
                registry.createEntity()
                    .with<ecs::Transform>("enemy_transform", x, y, 0.0f)
                    .with<ecs::Velocity>("enemy_velocity", -GameConfig::Enemy::Easy::SPEED, 0.0f)
                    .with<ecs::Rect>("enemy_rect", 0.0f, 0.0f, 
                                   static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH), 
                                   static_cast<int>(GameConfig::Enemy::Easy::SPRITE_HEIGHT))
                    .with<ecs::Scale>("enemy_scale", GameConfig::Enemy::Easy::SCALE, GameConfig::Enemy::Easy::SCALE)
                    .with<ecs::Texture>("enemy_texture", Path::Texture::TEXTURE_ENEMY_EASY)
                    .with<ecs::Animation>("enemy_animation", 0, GameConfig::Enemy::Easy::ANIMATION_FRAMES, 
                                        GameConfig::Enemy::Easy::ANIMATION_DURATION, 0.0f,
                                        GameConfig::Enemy::Easy::SPRITE_WIDTH, GameConfig::Enemy::Easy::SPRITE_HEIGHT,
                                        GameConfig::Enemy::Easy::FRAMES_PER_ROW)
                        .with<ecs::Enemy>("enemy", GameConfig::Enemy::Easy::HEALTH, 
                                        GameConfig::Enemy::Easy::HEALTH, GameConfig::Enemy::Easy::DAMAGE,
                                        GameConfig::Enemy::Easy::SPEED, 0.0f, GameConfig::Enemy::Easy::SHOOT_COOLDOWN)
                        .with<ecs::Hitbox>("enemy_hitbox", GameConfig::Hitbox::ENEMY_RADIUS)
                        .build();
            }

            void spawnAsteroid(ecs::Registry &registry, ecs::Asteroid::Size size)
            {
                float x = GameConfig::Screen::SPAWN_X;
                float y = static_cast<float>(GameConfig::Screen::MIN_Y + (std::rand() % static_cast<int>(GameConfig::Screen::MAX_Y - GameConfig::Screen::MIN_Y)));
                
                registry.createEntity()
                    .with<ecs::Transform>("asteroid_transform", x, y, 0.0f)
                    .with<ecs::Velocity>("asteroid_velocity", -GameConfig::Asteroid::Small::SPEED, 0.0f)
                    .with<ecs::Rect>("asteroid_rect", 0.0f, 0.0f, 
                                   static_cast<int>(GameConfig::Asteroid::Small::SPRITE_WIDTH), 
                                   static_cast<int>(GameConfig::Asteroid::Small::SPRITE_HEIGHT))
                    .with<ecs::Scale>("asteroid_scale", GameConfig::Asteroid::Small::SCALE, GameConfig::Asteroid::Small::SCALE)
                    .with<ecs::Texture>("asteroid_texture", Path::Texture::TEXTURE_ASTEROID)
                    .with<ecs::Animation>("asteroid_animation", 0, GameConfig::Asteroid::Small::ANIMATION_FRAMES, 
                                        GameConfig::Asteroid::Small::ANIMATION_DURATION, 0.0f,
                                        GameConfig::Asteroid::Small::SPRITE_WIDTH, GameConfig::Asteroid::Small::SPRITE_HEIGHT,
                                        GameConfig::Asteroid::Small::FRAMES_PER_ROW)
                    .with<ecs::Asteroid>("asteroid", size, GameConfig::Asteroid::Small::ROTATION_SPEED, 
                                       GameConfig::Asteroid::Small::HEALTH)
                    .with<ecs::Hitbox>("asteroid_hitbox", GameConfig::Hitbox::ASTEROID_SMALL_RADIUS)
                    .build();
            }

            void spawnWave(ecs::Registry &registry)
            {
                int waveSize = 5 + (std::rand() % 4);
                
                for (int i = 0; i < waveSize; ++i)
                {
                    float x = GameConfig::Screen::SPAWN_X + (i * 100.0f);
                    float y = static_cast<float>(GameConfig::Screen::MIN_Y + (std::rand() % static_cast<int>(GameConfig::Screen::MAX_Y - GameConfig::Screen::MIN_Y)));
                    
                    registry.createEntity()
                        .with<ecs::Transform>("enemy_wave_transform", x, y, 0.0f)
                        .with<ecs::Velocity>("enemy_wave_velocity", -GameConfig::Enemy::Easy::SPEED, 0.0f)
                        .with<ecs::Rect>("enemy_wave_rect", 0.0f, 0.0f, 
                                       static_cast<int>(GameConfig::Enemy::Easy::SPRITE_WIDTH), 
                                       static_cast<int>(GameConfig::Enemy::Easy::SPRITE_HEIGHT))
                        .with<ecs::Scale>("enemy_wave_scale", GameConfig::Enemy::Easy::SCALE, GameConfig::Enemy::Easy::SCALE)
                        .with<ecs::Texture>("enemy_wave_texture", Path::Texture::TEXTURE_ENEMY_EASY)
                        .with<ecs::Animation>("enemy_wave_animation", 0, GameConfig::Enemy::Easy::ANIMATION_FRAMES, 
                                            GameConfig::Enemy::Easy::ANIMATION_DURATION, 0.0f,
                                            GameConfig::Enemy::Easy::SPRITE_WIDTH, GameConfig::Enemy::Easy::SPRITE_HEIGHT,
                                            GameConfig::Enemy::Easy::FRAMES_PER_ROW)
                        .with<ecs::Enemy>("enemy_wave", GameConfig::Enemy::Easy::HEALTH, 
                                        GameConfig::Enemy::Easy::HEALTH, GameConfig::Enemy::Easy::DAMAGE,
                                        GameConfig::Enemy::Easy::SPEED, 0.0f, GameConfig::Enemy::Easy::SHOOT_COOLDOWN)
                        .with<ecs::Hitbox>("enemy_wave_hitbox", GameConfig::Hitbox::ENEMY_RADIUS)
                        .build();
                }
            }
    }; // class SpawnSystem

    class ExplosionSystem final : public eng::ISystem
    {
        public:
            explicit ExplosionSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~ExplosionSystem() override = default;

            ExplosionSystem(const ExplosionSystem &) = delete;
            ExplosionSystem &operator=(const ExplosionSystem &) = delete;
            ExplosionSystem(ExplosionSystem &&) = delete;
            ExplosionSystem &operator=(ExplosionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> explosionsToRemove;
                
                for (auto &[entity, explosion] : registry.getAll<ecs::Explosion>())
                {
                    auto *transform = registry.getComponent<ecs::Transform>(entity);
                    auto *rect = registry.getComponent<ecs::Rect>(entity);
                    auto *texture = registry.getComponent<ecs::Texture>(entity);
                    auto *scale = registry.getComponent<ecs::Scale>(entity);
                    
                    if (!transform || !rect || !texture || !scale) continue;

                    explosion.current_time += dt;
                    if (explosion.current_time >= explosion.frame_duration)
                    {
                        explosion.current_time = 0.0f;
                        explosion.current_frame = (explosion.current_frame + 1) % explosion.total_frames;
                        
                        int frame_x = (explosion.current_frame % explosion.frames_per_row) * static_cast<int>(explosion.frame_width);
                        int frame_y = (explosion.current_frame / explosion.frames_per_row) * static_cast<int>(explosion.frame_height);
                        
                        rect->pos_x = static_cast<float>(frame_x);
                        rect->pos_y = static_cast<float>(frame_y);
                    }

                    m_renderer.createSprite(texture->id + std::to_string(entity), texture->path, 
                                          transform->x, transform->y, scale->x, scale->y,
                                          static_cast<int>(rect->pos_x), static_cast<int>(rect->pos_y),
                                          static_cast<int>(rect->size_x), static_cast<int>(rect->size_y));
                    m_renderer.drawSprite(texture->id + std::to_string(entity));

                    explosion.current_lifetime += dt;
                    if (explosion.current_lifetime >= explosion.lifetime)
                    {
                        explosionsToRemove.push_back(entity);
                    }
                }
                
                for (ecs::Entity entity : explosionsToRemove)
                {
                    removeExplosion(registry, entity);
                }
            }

        private:
            eng::IRenderer &m_renderer;
            
            void removeExplosion(ecs::Registry &registry, ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Explosion>(entity))
                    registry.removeComponent<ecs::Explosion>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
            }
    };

    class CollisionSystem final : public eng::ISystem
    {
        public:
            explicit CollisionSystem(eng::IRenderer &renderer) : m_renderer(renderer) {}
            ~CollisionSystem() override = default;

            CollisionSystem(const CollisionSystem &) = delete;
            CollisionSystem &operator=(const CollisionSystem &) = delete;
            CollisionSystem(CollisionSystem &&) = delete;
            CollisionSystem &operator=(CollisionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::vector<ecs::Entity> projectilesToRemove;
                std::vector<ecs::Entity> enemiesToRemove;
                std::vector<ecs::Entity> asteroidsToRemove;

                for (auto &[projectileEntity, projectile] : registry.getAll<ecs::Projectile>())
                {
                    auto *projectileTransform = registry.getComponent<ecs::Transform>(projectileEntity);
                    auto *projectileHitbox = registry.getComponent<ecs::Hitbox>(projectileEntity);
                    if (!projectileTransform || !projectileHitbox) continue;

                    for (auto &[enemyEntity, enemy] : registry.getAll<ecs::Enemy>())
                    {
                        auto *enemyTransform = registry.getComponent<ecs::Transform>(enemyEntity);
                        auto *enemyHitbox = registry.getComponent<ecs::Hitbox>(enemyEntity);
                        if (!enemyTransform || !enemyHitbox) continue;

                        if (checkCircularCollision(*projectileTransform, *projectileHitbox, 
                                                  *enemyTransform, *enemyHitbox))
                        {
                            enemy.health -= projectile.damage;

                            projectilesToRemove.push_back(projectileEntity);

                            if (enemy.health <= 0.0f)
                            {
                                createExplosion(registry, enemyTransform->x, enemyTransform->y);
                                enemiesToRemove.push_back(enemyEntity);
                            }
                            break;
                        }
                    }
                }

                for (auto &[projectileEntity, projectile] : registry.getAll<ecs::Projectile>())
                {
                    auto *projectileTransform = registry.getComponent<ecs::Transform>(projectileEntity);
                    auto *projectileHitbox = registry.getComponent<ecs::Hitbox>(projectileEntity);
                    if (!projectileTransform || !projectileHitbox) continue;

                    for (auto &[asteroidEntity, asteroid] : registry.getAll<ecs::Asteroid>())
                    {
                        auto *asteroidTransform = registry.getComponent<ecs::Transform>(asteroidEntity);
                        auto *asteroidHitbox = registry.getComponent<ecs::Hitbox>(asteroidEntity);
                        if (!asteroidTransform || !asteroidHitbox) continue;

                        if (checkCircularCollision(*projectileTransform, *projectileHitbox, 
                                                  *asteroidTransform, *asteroidHitbox))
                        {
                            asteroid.health -= projectile.damage;

                            projectilesToRemove.push_back(projectileEntity);

                            if (asteroid.health <= 0.0f)
                            {
                                createExplosion(registry, asteroidTransform->x, asteroidTransform->y);
                                asteroidsToRemove.push_back(asteroidEntity);
                            }
                            break;
                        }
                    }
                }

                for (ecs::Entity entity : projectilesToRemove)
                {
                    removeProjectile(registry, entity);
                }
                for (ecs::Entity entity : enemiesToRemove)
                {
                    removeEnemy(registry, entity);
                }
                for (ecs::Entity entity : asteroidsToRemove)
                {
                    removeAsteroid(registry, entity);
                }
            }

        private:
            eng::IRenderer &m_renderer;

            bool checkCircularCollision(const ecs::Transform &transform1, const ecs::Hitbox &hitbox1,
                                      const ecs::Transform &transform2, const ecs::Hitbox &hitbox2)
            {
                float dx = transform1.x - transform2.x;
                float dy = transform1.y - transform2.y;
                float distance = std::sqrt(dx * dx + dy * dy);
                float combinedRadius = hitbox1.radius + hitbox2.radius;
                
                return distance < combinedRadius;
            }

            void removeProjectile(ecs::Registry &registry, ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Projectile>(entity))
                    registry.removeComponent<ecs::Projectile>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Velocity>(entity))
                    registry.removeComponent<ecs::Velocity>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
                if (registry.hasComponent<ecs::Animation>(entity))
                    registry.removeComponent<ecs::Animation>(entity);
                if (registry.hasComponent<ecs::Hitbox>(entity))
                    registry.removeComponent<ecs::Hitbox>(entity);
            }

            void removeEnemy(ecs::Registry &registry, ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Enemy>(entity))
                    registry.removeComponent<ecs::Enemy>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Velocity>(entity))
                    registry.removeComponent<ecs::Velocity>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
                if (registry.hasComponent<ecs::Animation>(entity))
                    registry.removeComponent<ecs::Animation>(entity);
                if (registry.hasComponent<ecs::Hitbox>(entity))
                    registry.removeComponent<ecs::Hitbox>(entity);
            }

            void removeAsteroid(ecs::Registry &registry, ecs::Entity entity)
            {
                if (registry.hasComponent<ecs::Asteroid>(entity))
                    registry.removeComponent<ecs::Asteroid>(entity);
                if (registry.hasComponent<ecs::Transform>(entity))
                    registry.removeComponent<ecs::Transform>(entity);
                if (registry.hasComponent<ecs::Velocity>(entity))
                    registry.removeComponent<ecs::Velocity>(entity);
                if (registry.hasComponent<ecs::Rect>(entity))
                    registry.removeComponent<ecs::Rect>(entity);
                if (registry.hasComponent<ecs::Texture>(entity))
                    registry.removeComponent<ecs::Texture>(entity);
                if (registry.hasComponent<ecs::Scale>(entity))
                    registry.removeComponent<ecs::Scale>(entity);
                if (registry.hasComponent<ecs::Hitbox>(entity))
                    registry.removeComponent<ecs::Hitbox>(entity);
            }

            void createExplosion(ecs::Registry &registry, float x, float y)
            {
                registry.createEntity()
                    .with<ecs::Transform>("explosion_transform", x, y, 0.0f)
                    .with<ecs::Rect>("explosion_rect", 0.0f, 0.0f, 
                                   static_cast<int>(GameConfig::Explosion::SPRITE_WIDTH), 
                                   static_cast<int>(GameConfig::Explosion::SPRITE_HEIGHT))
                    .with<ecs::Scale>("explosion_scale", GameConfig::Explosion::SCALE, GameConfig::Explosion::SCALE)
                    .with<ecs::Texture>("explosion_texture", Path::Texture::TEXTURE_EXPLOSION)
                    .with<ecs::Explosion>("explosion", 0, GameConfig::Explosion::ANIMATION_FRAMES, 
                                        GameConfig::Explosion::ANIMATION_DURATION, 0.0f,
                                        GameConfig::Explosion::SPRITE_WIDTH, GameConfig::Explosion::SPRITE_HEIGHT,
                                        GameConfig::Explosion::FRAMES_PER_ROW, GameConfig::Explosion::LIFETIME, 0.0f)
                    .build();
            }
    }; // class CollisionSystem

} // namespace cli