///
/// @file AGamePlugin.hpp
/// @brief Base class for R-Type game plugins
/// @namespace gme
///

#pragma once

#include <memory>
#include <string>

#include "Interfaces/IGameClient.hpp"
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
    /// @class AGamePlugin
    /// @brief Abstract base class for R-Type game plugins
    /// @namespace gme
    ///
    class AGamePlugin : public IGameClient
    {
        public:
            ~AGamePlugin() override = default;

            // IGameClient interface implementation
            void initialize(const std::shared_ptr<eng::IRenderer> &renderer, 
                          const std::shared_ptr<eng::IAudio> &audio) override;
            void update(float deltaTime, unsigned int width, unsigned int height) override;
            void handleEvent(const eng::Event &event) override;
            [[nodiscard]] const IScene &getCurrentScene() const override { return *m_currentScene; }
            [[nodiscard]] eng::IScene *getCurrentEngineScene() const override;

            [[nodiscard]] std::string &getGameName() override { return m_name; }
            void setName(const std::string &newName) override { m_name = newName; }

        protected:
            // Protected methods for derived classes
            virtual void initializeScenes() = 0;
            void convertEntitiesToSprites();

            // Protected members
            std::string m_name = "R-Type";
            std::shared_ptr<RTypeGameScene> m_currentScene;
            std::shared_ptr<eng::IRenderer> m_renderer;
            std::shared_ptr<eng::IAudio> m_audio;
            std::unique_ptr<eng::SceneManager> m_sceneManager;
    }; // class AGamePlugin

} // namespace gme

