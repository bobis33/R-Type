///
/// @file RTypeGame.hpp
/// @brief R-Type Game Plugin implementation
/// @namespace gme
///

#pragma once

#include <memory>
#include <unordered_map>
#include <functional>

#include "Interfaces/IGameClient.hpp"
#include "Engine/Interfaces/IScene.hpp"
#include "Engine/SceneManager.hpp"
#include "Interfaces/IAudio.hpp"
#include "Interfaces/IRenderer.hpp"

namespace gme
{
    ///
    /// @class RTypeGameScene
    /// @brief Implementation of IScene for RType game
    /// @namespace gme
    ///
    class RTypeGameScene final : public IScene
    {
        public:
            RTypeGameScene(const std::string &name) : m_name(name) {}
            ~RTypeGameScene() override = default;

            [[nodiscard]] const std::string &getName() const override { return m_name; }
            [[nodiscard]] const std::vector<Sprite> &getEntities() const override { return m_entities; }
            [[nodiscard]] std::vector<Sprite> &getEntitiesMutable() override { return m_entities; }

            void addEntity(const Sprite &sprite) { m_entities.push_back(sprite); }
            void clearEntities() { m_entities.clear(); }

        private:
            std::string m_name;
            std::vector<Sprite> m_entities;
    }; // class RTypeGameScene

    ///
    /// @class RTypeGame
    /// @brief Main R-Type game plugin implementation
    /// @namespace gme
    ///
    class RTypeGame final : public IGameClient
    {
        public:
            RTypeGame();
            ~RTypeGame() override = default;

            RTypeGame(const RTypeGame &) = delete;
            RTypeGame &operator=(const RTypeGame &) = delete;
            RTypeGame(RTypeGame &&) = delete;
            RTypeGame &operator=(RTypeGame &&) = delete;

            // IGameClient interface
            [[nodiscard]] std::string &getGameName() override { return m_name; }
            void setName(const std::string &newName) override { m_name = newName; }
            void update(float deltaTime, unsigned int width, unsigned int height) override;
            [[nodiscard]] const IScene &getCurrentScene() const override { return *m_currentScene; }

            // Game-specific methods
            void initialize(const std::shared_ptr<eng::IRenderer> &renderer, 
                          const std::shared_ptr<eng::IAudio> &audio) override;
            void handleEvent(const eng::Event &event) override;
            [[nodiscard]] eng::IScene *getCurrentEngineScene() const override;
            [[nodiscard]] eng::SceneManager &getSceneManager() { return *m_sceneManager; }

        private:
            void initializeScenes();
            void convertEntitiesToSprites();

            std::string m_name = "R-Type";
            std::shared_ptr<RTypeGameScene> m_currentScene;
            std::shared_ptr<eng::IRenderer> m_renderer;
            std::shared_ptr<eng::IAudio> m_audio;
            
            // Scene manager for engine scenes (using ECS)
            std::unique_ptr<eng::SceneManager> m_sceneManager;
    }; // class RTypeGame

} // namespace gme

