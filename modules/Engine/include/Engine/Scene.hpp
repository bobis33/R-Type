///
/// @file Scene.hpp
/// @brief This file contains the Scene class
/// @namespace eng
///

#pragma once

#include <string>

namespace eng
{

    using scene_id_t = unsigned int;

    ///
    /// @class Scene
    /// @brief class for scene and manage entities
    /// @namespace eng
    ///
    class Scene
    {
        public:
            Scene() = default;
            ~Scene() = default;

            [[nodiscard]] std::string &getName() { return m_name; }
            void setName(const std::string &newName) { m_name = newName; }

            [[nodiscard]] bool getDisplay() const { return m_display; }
            void setDisplay(const bool display) { m_display = display; }

            [[nodiscard]] scene_id_t getId() const { return m_id; }

        private:
            std::string m_name = "default_name";
            bool m_display = false;
            scene_id_t m_id = 0;
    }; // class Scene

} // namespace eng