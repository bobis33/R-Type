///
/// @file Settings.hpp
/// @brief Declaration of the Settings scene (game configuration)
/// @namespace cli
///

#pragma once

#include "Engine/IScene.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include "ECS/Entity.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace cli
{
    class Settings final : public eng::AScene
    {
    public:
        Settings(const std::unique_ptr<eng::IRenderer> &renderer,
                 const std::unique_ptr<eng::IAudio> &audio);
        ~Settings() override = default;

        Settings(const Settings &) = delete;
        Settings &operator=(const Settings &) = delete;
        Settings(Settings &&) = delete;
        Settings &operator=(Settings &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;
        bool shouldReturnMenu() const { return m_returnMenu; }

    private:
        struct Item
        {
            ecs::Entity entity;
            std::string id;
        };

        void createSettingsEntities();
        void updateHighlight();

        std::vector<Item> m_items;
        int m_selectedIndex = 0;
        bool m_returnMenu = false;
        int m_volume = 50;
        int m_fps = 240;
        bool m_fullscreen = false;

        eng::IRenderer &m_renderer;
        eng::IAudio &m_audio;
    };
} // namespace cli
