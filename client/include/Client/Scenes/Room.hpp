///
/// @file Room.hpp
/// @brief Multiplayer Room scene
///

#pragma once

#include "Engine/IScene.hpp"
#include "Engine/SceneManager.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include <vector>
#include <string>

namespace cli
{
    class Room final : public eng::AScene
    {
    ///
    /// @file Room.hpp
    /// @brief Multiplayer Room scene
    ///
    public:
        Room(const std::shared_ptr<eng::IRenderer> &renderer,
             const std::shared_ptr<eng::IAudio> &audio,
             eng::SceneManager *sceneManager,
             bool isHost);
        ~Room() override = default;

        Room(const Room &) = delete;
        Room &operator=(const Room &) = delete;
        Room(Room &&) = delete;
        Room &operator=(Room &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

    private:
        struct Item {
            ecs::Entity entity;
            std::string id;
        };

        void createRoomEntities();
        void updateHighlight();

        std::vector<Item> m_items;
        int m_selectedIndex = 0;
        bool m_isHost = false;

        eng::SceneManager *m_sceneManager = nullptr; // 🔁 gestion des transitions
        ecs::Entity m_titleEntity{};
    };
} // namespace cli
