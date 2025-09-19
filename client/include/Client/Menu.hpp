#pragma once

#include "Engine/Engine.hpp"
#include "Interfaces/IRenderer.hpp"
#include <vector>   // ⚡ nécessaire

namespace cli
{
    class Menu
    {
    public:
        explicit Menu(eng::Engine &engine);
        ~Menu() = default;

        void handleEvent(const eng::Event &event);
        void update(float dt);
        void unload(); // détruit les entités du menu

        bool shouldStartGame() const { return m_startGame; }
        bool shouldExitGame() const { return m_exitGame; }

    private:
        eng::Engine &m_engine;
        bool m_startGame = false;
        bool m_exitGame = false;

        std::vector<ecs::Entity> m_entities; // ⚡ liste des entités du menu
        void createMenuEntities();
    };
}
