///
/// @file Sprite.hpp
/// @brief This file contains the sprite system definition
/// @namespace ecs
///

#pragma once

#include <algorithm>
#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace ecs
{

    ///
    /// @class SpriteSystem
    /// @brief Class for sprite system
    /// @namespace ecs
    ///
    class SpriteSystem final : public ASystem
    {
        public:
            explicit SpriteSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~SpriteSystem() override = default;

            SpriteSystem(const SpriteSystem &) = delete;
            SpriteSystem &operator=(const SpriteSystem &) = delete;
            SpriteSystem(SpriteSystem &&) = delete;
            SpriteSystem &operator=(SpriteSystem &&) = delete;

            void update(Registry &registry, float /* dt */) override
            {
                std::vector<std::pair<Entity, int>> spritesWithLayers;
                for (const auto &entity : registry.getAll<Texture>() | std::views::keys)
                {
                    const auto *layer = registry.getComponent<Layer>(entity);
                    int layerValue = (layer != nullptr) ? layer->layer : 0;
                    spritesWithLayers.emplace_back(entity, layerValue);
                }
                std::ranges::sort(spritesWithLayers,
                          [](const auto &a, const auto &b) { return a.second < b.second; });
                for (const auto &entity : spritesWithLayers | std::views::keys)
                {
                    const auto *sprite = registry.getComponent<Texture>(entity);
                    const auto *transform = registry.getComponent<Transform>(entity);
                    const auto *rect = registry.getComponent<Rect>(entity);
                    const auto *scale = registry.getComponent<Scale>(entity);
                    const bool hasScrolling = registry.hasComponent<Scrolling>(entity);

                    const float x = (transform != nullptr) ? transform->x : 0.F;
                    const float y = (transform != nullptr) ? transform->y : 0.F;
                    m_renderer->setSpriteTexture(sprite->id + std::to_string(entity), sprite->path);
                    m_renderer->setSpritePosition(sprite->id + std::to_string(entity), x, y);
                    if ((scale != nullptr) && !hasScrolling)
                    {
                        m_renderer->setSpriteScale(sprite->id + std::to_string(entity), static_cast<int>(scale->x),
                                                   static_cast<int>(scale->y));
                    }
                    if (rect != nullptr)
                    {
                        m_renderer->setSpriteFrame(sprite->id + std::to_string(entity), static_cast<int>(rect->pos_x),
                                                   static_cast<int>(rect->pos_y), rect->size_x, rect->size_y);
                    }
                    m_renderer->drawSprite(sprite->id + std::to_string(entity));
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class SpriteSystem
} // namespace ecs