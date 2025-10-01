///
/// @file Lobby.hpp
/// @brief Multiplayer lobby scene (room management)
///

#pragma once

#include "Engine/IScene.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include <vector>
#include <string>

namespace cli
{
    class Lobby final : public eng::AScene
    {
    public:
        Lobby(const std::unique_ptr<eng::IRenderer> &renderer,
              const std::unique_ptr<eng::IAudio> &audio);
        ~Lobby() override = default;

        Lobby(const Lobby &) = delete;
        Lobby &operator=(const Lobby &) = delete;
        Lobby(Lobby &&) = delete;
        Lobby &operator=(Lobby &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

        bool shouldReturnMenu() const { return m_returnMenu; }
        bool shouldStartRoom() const { return m_startRoom; }

    private:
        struct Room {
            ecs::Entity entity;
            std::string id;
        };

        std::vector<Room> m_rooms;
        int m_selectedIndex = 0;

        bool m_returnMenu = false;
        bool m_startRoom = false;

        void createLobbyEntities();
        void updateHighlight();
    };
} // namespace cli
