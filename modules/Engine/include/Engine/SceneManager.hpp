///
/// @file SceneManager.hpp
/// @brief This file contains the SceneManager class declaration
/// @namespace eng
///

#pragma once

#include <memory>
#include <unordered_map>

#include "Engine/Scene.hpp"

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

            std::unique_ptr<Scene> &getScene(scene_id_t sceneId);
            std::unique_ptr<Scene> &getCurrentScene();
            scene_id_t addScene(std::unique_ptr<Scene> scene);
            void switchToScene(scene_id_t sceneId);

        private:
            std::unordered_map<scene_id_t, std::unique_ptr<Scene>> m_scenes;
            scene_id_t m_currentSceneId = 0;

    }; // class SceneManager

} // namespace eng