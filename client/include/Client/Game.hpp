///
/// @file Game.hpp
/// @brief Declaration of the Game class
/// @namespace cli
///

#pragma once

#include "Engine/Engine.hpp"
#include "Interfaces/IRenderer.hpp"

namespace cli
{
    class Game
    {
    public:
        explicit Game(eng::Engine &engine);
        ~Game() = default;

        Game(const Game &) = delete;
        Game &operator=(const Game &) = delete;
        Game(Game &&) = delete;
        Game &operator=(Game &&) = delete;

        void handleEvent(const eng::Event &event);
        void update(float dt);

    private:
        eng::Engine &m_engine;
        ecs::Entity m_fpsEntity;
    };
}
