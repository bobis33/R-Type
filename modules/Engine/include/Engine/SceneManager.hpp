///
/// @file SceneManager.hpp
/// @brief This file contains the SceneManager class declaration
/// @namespace eng
///

#pragma once

#include <memory>
#include <unordered_map>
#include "Engine/IScene.hpp"
#include "Engine/SettingsManager.hpp"

namespace eng
{

    ///
    /// @class SceneManager
    /// @brief Class for managing scenes
    /// @namespace eng
    ///
    class SceneManager
    {

        public:
            SceneManager() = default;
            ~SceneManager() = default;

            SceneManager(const SceneManager &) = delete;
            SceneManager &operator=(const SceneManager &) = delete;
            SceneManager(SceneManager &&) = delete;
            SceneManager &operator=(SceneManager &&) = delete;

            std::unique_ptr<IScene> &getScene(const id sceneId) { return m_scenes.at(sceneId); }
            std::unique_ptr<IScene> &getCurrentScene() { return m_scenes.at(m_currentSceneId); }
            void switchToScene(const id sceneId) { m_currentSceneId = sceneId; }
            void addScene(std::unique_ptr<IScene> scene) { m_scenes[scene->getId()] = std::move(scene); }

            // template <typename... EntityDefs>
            // IScene &createScene(const std::string &name, const std::function<void(const Event&)> eventHandler, const
            // std::function<void(float)> updateHandler,
            //                    EntityDefs&&... defs) {
            //     auto scene = std::make_unique<IScene>();
            //     scene->setName(name);
            //     scene->setEventHandler(eventHandler);
            //     scene->setUpdateHandler(updateHandler);
            //
            //     Scene &ref = *scene;
            //     (defs(ref.getRegistry()), ...);
            //
            //     m_scenes[ref.getId()] = std::move(scene);
            //     return ref;
            // }

            void setSettingsManager(std::shared_ptr<SettingsManager> settings) { m_settings = std::move(settings); }
            std::shared_ptr<SettingsManager> getSettingsManager() { return m_settings; }

        private:
            std::unordered_map<id, std::unique_ptr<IScene>> m_scenes;
            id m_currentSceneId = 1;
            std::shared_ptr<SettingsManager> m_settings;
    }; // class SceneManager
} // namespace eng