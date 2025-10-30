#pragma once

#include <cmath>
#include <ranges>
#include <vector>

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{
    class CollisionSystem final : public ecs::ASystem
    {
        public:
            explicit CollisionSystem(const std::shared_ptr<eng::IRenderer> &renderer, bool &showDebug)
                : m_renderer(renderer), m_showDebug(showDebug)
            {
            }
            ~CollisionSystem() override = default;

            CollisionSystem(const CollisionSystem &) = delete;
            CollisionSystem &operator=(const CollisionSystem &) = delete;
            CollisionSystem(CollisionSystem &&) = delete;
            CollisionSystem &operator=(const CollisionSystem &&) = delete;

            bool isEnable() override { return true; }
            void setEnable(bool enable) override { (void)enable; }

            void update(ecs::Registry &registry, float dt) override
            {
                std::optional<float> ceilingBottomY;
                std::optional<float> floorTopY;
                
                for (auto &pair : registry.getAll<ecs::Ceiling>())
                {
                    const auto entity = pair.first;
                    const auto *t = registry.getComponent<ecs::Transform>(entity);
                    const auto *s = registry.getComponent<ecs::Scale>(entity);
                    const auto *scroll = registry.getComponent<ecs::Scrolling>(entity);
                    if ((t == nullptr) || (scroll == nullptr))
                    {
                        continue;
                    }
                    const float scaledHeight = (s ? s->y : 1.0f) * scroll->original_height;
                    ceilingBottomY = t->y + scaledHeight;
                    break; // Un seul suffit
                }
                
                for (auto &pair : registry.getAll<ecs::Floor>())
                {
                    const auto entity = pair.first;
                    const auto *t = registry.getComponent<ecs::Transform>(entity);
                    if (t == nullptr)
                    {
                        continue;
                    }
                    floorTopY = t->y;
                    break; // Un seul suffit
                }

                if (ceilingBottomY.has_value() || floorTopY.has_value())
                {
                    // Optimisation: itération directe
                    for (auto &pair : registry.getAll<ecs::Player>())
                    {
                        const auto playerEntity = pair.first;
                        auto *t = registry.getComponent<ecs::Transform>(playerEntity);
                        auto *hb = registry.getComponent<ecs::Hitbox>(playerEntity);
                        auto *vel = registry.getComponent<ecs::Velocity>(playerEntity);
                        if (!t || !hb)
                        {
                            continue;
                        }
                        
                        float hitboxY = t->y + hb->offsetY;
                        
                        if (ceilingBottomY.has_value() && (hitboxY - hb->radius < ceilingBottomY.value()))
                        {
                            t->y = ceilingBottomY.value() + hb->radius - hb->offsetY;
                            if (vel != nullptr)
                            {
                                vel->y = std::max(0.0f, vel->y);
                            }
                        }
                        
                        if (floorTopY.has_value() && (hitboxY + hb->radius > floorTopY.value()))
                        {
                            t->y = floorTopY.value() - hb->radius - hb->offsetY;
                            if (vel != nullptr)
                            {
                                vel->y = std::min(0.0f, vel->y);
                            }
                        }
                    }
                }
            }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            bool &m_showDebug;
    };
} // namespace gme

