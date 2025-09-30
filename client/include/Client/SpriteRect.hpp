///
/// @file SpriteRect.hpp
/// @brief Helper functions for pixel-perfect sprite rectangles
/// @namespace cli
///

#pragma once

namespace cli
{
    namespace SpriteRect
    {
        // SpriteSheet configuration for pixel-perfect rendering
        namespace SpriteSheet
        {
            // Asteroid spritesheet (r-typesheet3.gif)
            struct Asteroid
            {
                static constexpr int frameW = 16;
                static constexpr int frameH = 16;
                static constexpr int marginX = 1;
                static constexpr int marginY = 1;
                static constexpr int spacingX = 1;
                static constexpr int spacingY = 0;
                static constexpr int totalFrames = 12;
            };

            // Enemy spritesheet (r-typesheet5.gif)
            struct Enemy
            {
                static constexpr int frameW = 16;
                static constexpr int frameH = 16;
                static constexpr int marginX = 1;
                static constexpr int marginY = 1;
                static constexpr int spacingX = 1;
                static constexpr int spacingY = 0;
                static constexpr int totalFrames = 8;
            };
        } // namespace SpriteSheet

        // Simple rectangle structure
        struct Rect
        {
            int x, y, w, h;
        };

        // Calculate exact rectangle for asteroid frame
        inline Rect asteroidRect(int col, int row = 0)
        {
            const int fx = SpriteSheet::Asteroid::marginX + col * (SpriteSheet::Asteroid::frameW + SpriteSheet::Asteroid::spacingX);
            const int fy = SpriteSheet::Asteroid::marginY + row * (SpriteSheet::Asteroid::frameH + SpriteSheet::Asteroid::spacingY);
            return {fx, fy, SpriteSheet::Asteroid::frameW, SpriteSheet::Asteroid::frameH};
        }

        // Calculate exact rectangle for enemy frame
        inline Rect enemyRect(int col, int row = 0)
        {
            const int fx = SpriteSheet::Enemy::marginX + col * (SpriteSheet::Enemy::frameW + SpriteSheet::Enemy::spacingX);
            const int fy = SpriteSheet::Enemy::marginY + row * (SpriteSheet::Enemy::frameH + SpriteSheet::Enemy::spacingY);
            return {fx, fy, SpriteSheet::Enemy::frameW, SpriteSheet::Enemy::frameH};
        }
    } // namespace SpriteRect
} // namespace cli
