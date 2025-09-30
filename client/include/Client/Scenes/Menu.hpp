///
/// @file Menu.hpp
/// @brief Menu scene (main menu of the game)
///

#pragma once

#include "Engine/IScene.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include <vector>
#include <string>

namespace cli
{
    class Menu final : public eng::AScene
    {
    public:
        Menu(const std::unique_ptr<eng::IRenderer> &renderer,
             const std::unique_ptr<eng::IAudio> &audio);
        ~Menu() override = default;

        Menu(const Menu &) = delete;
        Menu &operator=(const Menu &) = delete;
        Menu(Menu &&) = delete;
        Menu &operator=(Menu &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

        bool shouldStartSolo() const { return m_startSolo; }
        bool shouldStartMulti() const { return m_startMulti; }
        bool shouldExitGame() const { return m_exitGame; }

    private:
        struct Item {
            ecs::Entity entity;
            std::string id;
        };

        std::vector<Item> m_items;
        int m_selectedIndex = 0;

        bool m_startSolo = false;
        bool m_startMulti = false;
        bool m_exitGame = false;

        void createMenuEntities();
        void updateHighlight();
    };
} // namespace cli
