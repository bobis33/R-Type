///
/// @file IGameClient.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include <vector>

namespace gme
{

    struct Sprite { // TODO(bobis33): should have path to texture, and all necessary data
        std::string type;
        float pos_x = 0, pos_y = 0;
        float v_x = 0, v_y = 0;
        float scale_x = 1, scale_y = 1;
        int r = -1, g = -1, b = -1, a = -1;
        std::string texture_path = "";
        int text_rect_x = 0, text_rect_y = 0, text_rect_fx = 0, text_rect_fy = 0;
        std::string id;
    };


    ///
    /// @class IScene
    /// @brief Interface for scenes
    /// @namespace cli
    ///
    class IScene {
        public:
            virtual ~IScene() = default;

            [[nodiscard]] virtual const std::string& getName() const = 0;
            [[nodiscard]] virtual const std::vector<Sprite>& getEntities() const = 0;
            [[nodiscard]] virtual std::vector<Sprite>& getEntitiesMutable() = 0;
    }; // class IScene

    ///
    /// @class IGameClient
    /// @brief Interface for the games
    /// @namespace gme
    ///
    class IGameClient
    {
        public:
            virtual ~IGameClient() = default;

            [[nodiscard]] virtual std::string &getName() = 0;
            virtual void setName(const std::string &newName) = 0;

            virtual void update(float deltaTime, int width, int height) = 0;
            [[nodiscard]] virtual const IScene& getCurrentScene() const = 0;

        private:
    }; // class IGameClient

} // namespace gme