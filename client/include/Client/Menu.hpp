#pragma once

#include "Engine/Engine.hpp"
#include "Interfaces/IRenderer.hpp"
#include <vector>

namespace cli
{
    class Menu
    {
    public:
        explicit Menu(eng::Engine &engine);
        ~Menu() = default;

        void handleEvent(const eng::Event &event);
        void update(float dt);
        void unload();
        bool shouldStartGame() const { return m_startGame; }
        bool shouldExitGame() const { return m_exitGame; }

    private:
        struct MenuItem {
            ecs::Entity entity;
            std::string id;
        };
        eng::Engine &m_engine;
        bool m_startGame = false;
        bool m_exitGame = false;
        int m_selectedIndex = 0;
        std::vector<MenuItem> m_items;

        void createMenuEntities();
        void updateHighlight();
    };
}
