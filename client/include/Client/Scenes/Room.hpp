///
/// @file Room.hpp
/// @brief Multiplayer Room scene
///

#pragma once

#include "Engine/IScene.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include <vector>
#include <string>

namespace cli
{
    class Room final : public eng::AScene
    {
    public:
        Room(const std::unique_ptr<eng::IRenderer> &renderer,
             const std::unique_ptr<eng::IAudio> &audio,
             bool isHost);
        ~Room() override = default;

        Room(const Room &) = delete;
        Room &operator=(const Room &) = delete;
        Room(Room &&) = delete;
        Room &operator=(Room &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

        bool shouldStartGame() const { return m_startGame; }
        bool shouldReturnMenu() const { return m_backToMenu; }

    private:
        struct Item {
            ecs::Entity entity;
            std::string id;
        };

        std::vector<Item> m_items;
        int m_selectedIndex = 0;
        bool m_startGame = false;
        bool m_backToMenu = false;
        bool m_isHost = false;

        void createRoomEntities();
        void updateHighlight();
    };
} // namespace cli
