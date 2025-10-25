///
/// @file StageManager.hpp
/// @brief Stage Manager for handling floor and ceiling spawning
/// @namespace gme
///

#pragma once

#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    class StageManager
    {
        public:
            StageManager() = default;
            ~StageManager() = default;

            StageManager(const StageManager &) = delete;
            StageManager &operator=(const StageManager &) = delete;
            StageManager(StageManager &&) = delete;
            StageManager &operator=(StageManager &&) = delete;

            void update(ecs::Registry &registry, float dt, const eng::WindowSize &size);
            void spawnStage(ecs::Registry &registry, int screenWidth);
            bool isStageSpawned() const { return m_stageSpawned; }

        private:
            float m_stageTimer = 0.0f;
            bool m_stageSpawned = false;
            static constexpr float STAGE_SPAWN_DELAY = 12.0f;
            static constexpr float SCROLL_SPEED = -40.0f;
            static constexpr float ORIGINAL_WIDTH = 2608.0f;
            static constexpr float SPRITE_HEIGHT = 208.0f;
    }; // class StageManager

} // namespace gme
