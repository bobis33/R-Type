///
/// @file IGameClient.hpp
/// @brief This file contains the Game interface
/// @namespace gme
///

#pragma once

#include <string>
#include <vector>

namespace gme
{

    struct Sprite
    { // TODO(bobis33): should have path to texture, and all necessary data
            std::string type;
            float pos_x = 0.F, pos_y = 0.F;
            float v_x = 0.F, v_y = 0.F;
            float scale_x = 1.F, scale_y = 1.F;
            unsigned char r = 255u, g = 255u, b = 255u, a = 255u;
            std::string texture_path = "";
            float text_rect_x = 0.F, text_rect_y = 0.F;
            int text_rect_fx = 0, text_rect_fy = 0;
            std::string id;
    };

    ///
    /// @class IScene
    /// @brief Interface for scenes
    /// @namespace gme
    ///
    class IScene
    {
        public:
            virtual ~IScene() = default;

            [[nodiscard]] virtual const std::string &getName() const = 0;
            [[nodiscard]] virtual const std::vector<Sprite> &getEntities() const = 0;
            [[nodiscard]] virtual std::vector<Sprite> &getEntitiesMutable() = 0;
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

            virtual void update(float deltaTime, unsigned int width, unsigned int height) = 0;
            [[nodiscard]] virtual const IScene &getCurrentScene() const = 0;

        private:
    }; // class IGameClient

} // namespace gme