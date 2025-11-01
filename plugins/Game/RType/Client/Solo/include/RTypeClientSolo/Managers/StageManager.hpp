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
            static void spawnStage(ecs::Registry &registry, int screenWidth);
            bool isStageSpawned() const { return m_stageSpawned; }
            int getCurrentWave() const { return m_currentWave; }
            float getWaveTimer() const { return m_waveTimer; }
            float getTotalGameTime() const { return m_totalGameTimer; }

        private:
            float m_stageTimer = 0.0f;
            bool m_stageSpawned = false;

            // Wave tracking (synchronized with server)
            int m_currentWave = 0;
            float m_waveTimer = 0.0f;
            float m_totalGameTimer = 0.0f;

            static constexpr float STAGE_SPAWN_DELAY = 0.5f;
            static constexpr float WAVE_DURATION = 30.0f;
            static constexpr int TOTAL_WAVES = 3;
            static constexpr float SCROLL_SPEED = -40.0f;
            static constexpr float ORIGINAL_WIDTH = 2608.0f;
            static constexpr float SPRITE_HEIGHT = 208.0f;
    }; // class StageManager

} // namespace gme
