#pragma once

#include "ECS/Registry.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{

    enum class StageState
    {
        SCROLLING = 0,
        BOSS_PHASE = 1,
        COMPLETED = 2
    };

    class StageManager
    {
        public:
            StageManager() = default;
            ~StageManager() = default;

            StageManager(const StageManager &) = delete;
            StageManager &operator=(const StageManager &) = delete;
            StageManager(StageManager &&) = delete;
            StageManager &operator=(const StageManager &&) = delete;

            void update(ecs::Registry &registry, float dt, const eng::WindowSize &size);
            static void spawnStage(ecs::Registry &registry, int screenWidth);
            bool isStageSpawned() const { return m_stageSpawned; }
            StageState getStageState() const { return m_stageState; }
            bool isStageCompleted() const { return m_stageState == StageState::COMPLETED; }
            float getStageProgress() const;
            float getBossPhaseProgress() const;
            std::string getStageStatusText() const;
            
            void startBossPhase();
            void completeStage();

        private:
            float m_stageTimer = 0.0f;
            float m_bossTimer = 0.0f;
            bool m_stageSpawned = false;
            StageState m_stageState = StageState::SCROLLING;
            
            static constexpr float STAGE_SPAWN_DELAY = 12.0f;
            static constexpr float STAGE_DURATION = 60.0f;
            static constexpr float BOSS_PHASE_DURATION = 30.0f;
            static constexpr float SCROLL_SPEED = -40.0f;
            static constexpr float ORIGINAL_WIDTH = 2608.0f;
            static constexpr float SPRITE_HEIGHT = 208.0f;
    };
} // namespace gme

