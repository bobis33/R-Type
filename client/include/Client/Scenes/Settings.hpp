///
/// @file Settings.hpp
/// @brief Declaration of the Settings scene (game configuration)
/// @namespace cli
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
    class Settings final : public eng::AScene
    {
    public:
        Settings(const std::shared_ptr<eng::IRenderer> &renderer,
                 const std::shared_ptr<eng::IAudio> &audio,
                 eng::SceneManager *sceneManager);
        ~Settings() override = default;

        Settings(const Settings &) = delete;
        Settings &operator=(const Settings &) = delete;
        Settings(Settings &&) = delete;
        Settings &operator=(Settings &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

    private:
        struct Item {
            ecs::Entity entity;
            std::string id;
        };

        void createSettingsEntities();
        void updateHighlight();

    std::vector<Item> m_items;
    int m_selectedIndex = 0;

    ecs::Entity m_titleEntity{};
    eng::IRenderer &m_renderer;
    eng::IAudio &m_audio;
    eng::SceneManager *m_sceneManager = nullptr;
    }; // class Settings
} // namespace cli
