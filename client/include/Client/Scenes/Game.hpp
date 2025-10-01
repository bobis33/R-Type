///
/// @file Game.hpp
/// @brief Declaration of the Game scene (solo gameplay)
/// @namespace cli
///

#pragma once

#include "Engine/IScene.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include <unordered_map>

namespace cli
{
    class Game final : public eng::AScene
    {
    public:
        Game(const std::unique_ptr<eng::IRenderer> &renderer,
             const std::unique_ptr<eng::IAudio> &audio);
        ~Game() override = default;

        Game(const Game &) = delete;
        Game &operator=(const Game &) = delete;
        Game(Game &&) = delete;
        Game &operator=(Game &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;
        bool shouldReturnMenu() const { return m_returnMenu; }

    private:
        std::unordered_map<eng::Key, bool> m_keysPressed;
        bool m_returnMenu = false;

        ecs::Entity m_playerEntity{};
        ecs::Entity m_fpsEntity{};
        eng::IRenderer &m_renderer;
    };
} // namespace cli
