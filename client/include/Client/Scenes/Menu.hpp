///
/// @file Menu.hpp
/// @brief Menu scene (main menu of the game)
/// @namespace cli
///

#pragma once

#include "Engine/IScene.hpp"
#include "Engine/SceneManager.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include "ECS/Entity.hpp"
#include <memory>
#include <string>
#include <vector>

namespace cli
{
    ///
    /// @file Menu.hpp
    /// @brief Menu scene (main menu of the game)
    /// @namespace cli
    ///
    class Menu final : public eng::AScene
    {
    public:
        Menu(const std::shared_ptr<eng::IRenderer> &renderer,
             const std::shared_ptr<eng::IAudio> &audio,
             eng::SceneManager *sceneManager);

        ~Menu() override = default;

        Menu(const Menu &) = delete;
        Menu &operator=(const Menu &) = delete;
        Menu(Menu &&) = delete;
        Menu &operator=(Menu &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

    private:
        struct Item
        {
            ecs::Entity entity;
            std::string id;
        };

        void createMenuEntities();
        void updateHighlight();

        ecs::Entity m_logoEntity = ecs::INVALID_ENTITY;
        std::vector<Item> m_items;
        int m_selectedIndex = 0;

        eng::SceneManager *m_sceneManager = nullptr;
    }; // class Menu
} // namespace cli
