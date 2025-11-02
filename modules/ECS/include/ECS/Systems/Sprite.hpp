///
/// @file Sprite.hpp
/// @brief This file contains the sprite system definition
/// @namespace ecs
///

#pragma once

#include <algorithm>
#include <ranges>
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
                spritesWithLayers.reserve(
                    registry.getAll<Texture>().size()); // Pré-allocation pour éviter réallocations

                for (auto &pair : registry.getAll<Texture>())
                {
                    const auto entity = pair.first;
                    const auto *layer = registry.getComponent<Layer>(entity);
                    int layerValue = (layer != nullptr) ? layer->layer : 0;
                    spritesWithLayers.emplace_back(entity, layerValue);
                }
                std::sort(spritesWithLayers.begin(), spritesWithLayers.end(),
                          [](const auto &a, const auto &b) { return a.second < b.second; });
                std::string spriteName;
                spriteName.reserve(64); // Taille typique attendue

                for (const auto &pair : spritesWithLayers)
                {
                    const auto entity = pair.first;
                    const auto *sprite = registry.getComponent<Texture>(entity);
                    if (!sprite)
                        continue;

                    const auto *transform = registry.getComponent<Transform>(entity);
                    const auto *rect = registry.getComponent<Rect>(entity);
                    const auto *scale = registry.getComponent<Scale>(entity);
                    const auto *color = registry.getComponent<Color>(entity);
                    const bool hasScrolling = registry.hasComponent<Scrolling>(entity);

                    const float x = (transform != nullptr) ? transform->x : 0.F;
                    const float y = (transform != nullptr) ? transform->y : 0.F;

                    spriteName.clear();
                    spriteName = sprite->id;
                    spriteName += std::to_string(entity);

                    m_renderer->setSpriteTexture(spriteName, sprite->path);
                    m_renderer->setSpritePosition(spriteName, x, y);
                    if ((scale != nullptr) && hasScrolling)
                    {
                        m_renderer->setSpriteScale(spriteName, scale->x, scale->y);
                    }
                    if (rect != nullptr)
                    {
                        m_renderer->setSpriteFrame(spriteName, static_cast<int>(rect->pos_x),
                                                   static_cast<int>(rect->pos_y), rect->size_x, rect->size_y);
                    }
                    if (color != nullptr)
                    {
                        m_renderer->setSpriteColor(sprite->id + std::to_string(entity),
                                                   {.r = color->r, .g = color->g, .b = color->b, .a = color->a});
                    }
                    m_renderer->drawSprite(spriteName);
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class SpriteSystem
} // namespace ecs
