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

    class SpriteSystem final : public eng::ASystem
    {
        public:
            explicit SpriteSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
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
                    m_renderer->setSpriteTexture(sprite.id + std::to_string(entity), sprite.path);
                    m_renderer->setSpritePosition(sprite.id + std::to_string(entity), x, y);

                    if (rect)
                    {
                        m_renderer->setSpriteFrame(sprite.id + std::to_string(entity), static_cast<int>(rect->pos_x),
                                                  static_cast<int>(rect->pos_y), rect->size_x, rect->size_y);
                    }

                    m_renderer->drawSprite(sprite.id + std::to_string(entity));
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
    }; // class SpriteSystem

} // namespace cli