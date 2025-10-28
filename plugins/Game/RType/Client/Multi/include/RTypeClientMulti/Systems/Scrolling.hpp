#pragma once

#include "ECS/Component.hpp"
#include "ECS/Interfaces/ISystems.hpp"
#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"
#include "RTypeShared/GameConfig.hpp"

namespace gme
{
    class StageManager; // Forward declaration
    
    class ScrollingSystem final : public ecs::ASystem
    {
        public:
            explicit ScrollingSystem(const std::shared_ptr<eng::IRenderer> &renderer) : m_renderer(renderer) {}
            ~ScrollingSystem() override = default;

            ScrollingSystem(const ScrollingSystem &) = delete;
            ScrollingSystem &operator=(const ScrollingSystem &) = delete;
            ScrollingSystem(ScrollingSystem &&) = delete;
            ScrollingSystem &operator=(const ScrollingSystem &&) = delete;

            void update(ecs::Registry &registry, float dt) override;
            void setStageManager(StageManager* stageManager) { m_stageManager = stageManager; }

        private:
            const std::shared_ptr<eng::IRenderer> &m_renderer;
            StageManager* m_stageManager = nullptr;
    };
} // namespace gme

