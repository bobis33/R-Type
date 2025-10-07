///
/// @file LobbyScene.hpp
/// @brief This file contains the lobby scene
/// @namespace eng
///

#pragma once

#include "Engine/IScene.hpp"
#include "Engine/SceneManager.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include <vector>
#include <string>
#include <memory>

namespace cli
{   
    ///
    /// @class Lobby
    /// @brief Lobby scene
    /// @namespace cli
    ///
    class Lobby final : public eng::AScene
    {
    public:
        Lobby(const std::shared_ptr<eng::IRenderer> &renderer,
              const std::shared_ptr<eng::IAudio> &audio,
              eng::SceneManager *sceneManager);
        ~Lobby() override = default;

        Lobby(const Lobby &) = delete;
        Lobby &operator=(const Lobby &) = delete;
        Lobby(Lobby &&) = delete;
        Lobby &operator=(Lobby &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

    private:
        struct RoomOption {
            ecs::Entity entity;
            std::string id;
        };

        void createLobbyEntities();
        void updateHighlight();

        std::vector<RoomOption> m_rooms;
        int m_selectedIndex = 0;

        eng::SceneManager *m_sceneManager = nullptr;
        ecs::Entity m_titleEntity{};
    }; // class Lobby
} // namespace cli
