///
/// @file Game.hpp
/// @brief Solo game scene
/// @namespace cli
///

#pragma once

#include "Engine/IScene.hpp"
#include "Interfaces/IRenderer.hpp"
#include "Interfaces/IAudio.hpp"
#include "Engine/SceneManager.hpp"
#include <unordered_map>
#include <memory>

namespace cli
{   
    ///
    /// @file Game.hpp
    /// @brief Solo game scene
    /// @namespace cli
    ///
    class Game final : public eng::AScene
    {
    public:
        Game(const std::shared_ptr<eng::IRenderer> &renderer,
             const std::shared_ptr<eng::IAudio> &audio,
             eng::SceneManager *sceneManager);
        ~Game() override = default;

        Game(const Game &) = delete;
        Game &operator=(const Game &) = delete;
        Game(Game &&) = delete;
        Game &operator=(Game &&) = delete;

        void update(float dt, const eng::WindowSize &size) override;
        void event(const eng::Event &event) override;

    private:
        eng::SceneManager *m_sceneManager = nullptr;

        std::unordered_map<eng::Key, bool> m_keysPressed;
        ecs::Entity m_playerEntity{};
        ecs::Entity m_fpsEntity{};
        eng::IRenderer &m_renderer;
    }; // class Game
} // namespace cli
