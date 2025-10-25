///
/// @file HitboxUtils.hpp
/// @brief Utility functions for hitbox calculations
/// @namespace cli
///

#pragma once

namespace cli
{
    namespace Utils
    {
        ///
        /// @brief Calculate the center position of a sprite
        /// @param x Sprite X position
        /// @param y Sprite Y position
        /// @param spriteWidth Sprite width
        /// @param spriteHeight Sprite height
        /// @param scale Sprite scale
        /// @return Pair of (centerX, centerY)
        ///
        inline std::pair<float, float> calculateSpriteCenter(float x, float y, 
                                                             float spriteWidth, float spriteHeight, 
                                                             float scale)
        {
            return {
                x + (spriteWidth * scale) / 2.0f,
                y + (spriteHeight * scale) / 2.0f
            };
        }

        ///
        /// @brief Calculate hitbox offsets to center the hitbox on the sprite
        /// @param spriteWidth Sprite width
        /// @param spriteHeight Sprite height
        /// @param scale Sprite scale
        /// @return Pair of (offsetX, offsetY)
        ///
        inline std::pair<float, float> calculateHitboxOffsets(float spriteWidth, float spriteHeight, 
                                                              float scale)
        {
            return {
                (spriteWidth * scale) / 2.0f,
                (spriteHeight * scale) / 2.0f
            };
        }

        ///
        /// @brief Calculate hitbox offsets relative to sprite position
        /// @param x Sprite X position
        /// @param y Sprite Y position
        /// @param spriteWidth Sprite width
        /// @param spriteHeight Sprite height
        /// @param scale Sprite scale
        /// @return Pair of (offsetX, offsetY) relative to sprite position
        ///
        inline std::pair<float, float> calculateHitboxOffsetsRelative(float x, float y,
                                                                      float spriteWidth, float spriteHeight, 
                                                                      float scale)
        {
            auto [centerX, centerY] = calculateSpriteCenter(x, y, spriteWidth, spriteHeight, scale);
            return {
                centerX - x,
                centerY - y
            };
        }
    } // namespace Utils
} // namespace cli
