///
/// @file SceneManager.hpp
/// @brief This file contains the SceneManager class declaration
/// @namespace eng
///

#pragma once

#include <memory>
#include <unordered_map>

#include "Engine/Interfaces/IScene.hpp"

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

        private:
            std::unordered_map<id, std::unique_ptr<IScene>> m_scenes;
            id m_currentSceneId = 1;
    }; // class SceneManager
} // namespace eng