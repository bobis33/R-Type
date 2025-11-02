///
/// @file Game.hpp
/// @brief This file contains the solo Game scene
/// @namespace gme
///

#pragma once

#include <memory>

#include "Engine/Interfaces/IScene.hpp"
#include "Interfaces/IAudio.hpp"

namespace gme
{

    ///
    /// @class Game
    /// @brief Game solo scene
    /// @namespace gme
    ///
    class Game final : public eng::AScene
    {
        public:
            Game(eng::id assignedId, const std::shared_ptr<eng::IRenderer> &renderer,
                 const std::shared_ptr<eng::IAudio> &audio, bool &showDebug, int skinIndex = 0,
                 const std::string &playerName = "Player");
            ~Game() override = default;

            Game(const Game &other) = delete;
            Game &operator=(const Game &other) = delete;
            Game(Game &&other) = delete;
            Game &operator=(Game &&other) = delete;

            void update(float dt, const eng::WindowSize &size) override;
            void event(const eng::Event &event) override;
            bool &playMusic() { return m_playMusic; }

        private:
            ecs::Entity createPlayer(ecs::Registry &registry);
            std::pair<ecs::Entity, ecs::Entity> createPipePair(ecs::Registry &registry, float x, float gapY) const;
            void resetGame();
            bool checkCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2) const;
            bool checkCircleRectCollision(float circleX, float circleY, float circleR, float rectX, float rectY, float rectW, float rectH) const;
            void checkCollisions(ecs::Registry &registry);
            void checkScore(ecs::Registry &registry);
            void drawDebugRectangles(ecs::Registry &registry) const;

            const std::shared_ptr<eng::IRenderer> &m_renderer;
            const std::shared_ptr<eng::IAudio> &m_audio;
            std::unordered_map<eng::Key, bool> m_keysPressed;
            ecs::Entity m_playerEntity;
            ecs::Entity m_playerNameEntity;
            ecs::Entity m_looseText;
            ecs::Entity m_flapSound;
            ecs::Entity m_looseSound;
            ecs::Entity m_scoreEntity;
            std::vector<std::pair<ecs::Entity, ecs::Entity>> m_pipes;
            std::vector<bool> m_pipeScored;
            bool &m_showDebug;
            bool m_playMusic = false;
            bool m_gameOver = false;
            bool m_gameOverShown = false;
            int m_score = 0;
            int m_skinIndex;
            std::string m_playerName;
    }; // class Game
} // namespace gme