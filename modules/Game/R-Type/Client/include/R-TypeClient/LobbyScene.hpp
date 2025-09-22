///
/// @file LobbyScene.hpp
/// @brief This file contains the lobby scene
/// @namespace gme
///

#pragma once

#include <string>
#include <vector>

#include "Interfaces/IGameClient.hpp"

namespace gme
{

    ///
    /// @class LobbyScene
    /// @brief Class for the Lobby scene
    /// @namespace gme
    ///
    class LobbyScene final : public IScene
    {
        public:
            LobbyScene() : m_name("Lobby") {}

            [[nodiscard]] const std::string &getName() const override { return m_name; }
            [[nodiscard]] const std::vector<Sprite> &getEntities() const override { return m_entities; }

            void addEntity(const Sprite &e) { m_entities.push_back(e); }
            std::vector<Sprite> &getEntitiesMutable() override { return m_entities; }

        private:
            std::string m_name;
            std::vector<Sprite> m_entities;
    }; // class LobbyScene

} // namespace gme
