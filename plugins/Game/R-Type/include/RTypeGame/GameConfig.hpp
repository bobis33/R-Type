///
/// @file GameConfig.hpp
/// @brief Configuration constants for the game
/// @namespace gme
///

#pragma once

namespace gme
{
    namespace GameConfig
    {
        namespace Screen
        {
            inline constexpr float SPAWN_X = 1950.0f;
            inline constexpr float MIN_Y = 50.0f;
            inline constexpr float MAX_Y = 1030.0f;
            inline constexpr float REMOVE_X = -100.0f;
            inline constexpr float REMOVE_MIN_Y = -50.0f;
            inline constexpr float REMOVE_MAX_Y = 1130.0f;
        } // namespace Screen
        namespace Player
        {
            inline constexpr float SPEED = 500.0f;
            inline constexpr float DIAGONAL_SPEED_MULTIPLIER = 0.707f;
            inline constexpr float SPRITE_WIDTH = 33.0f;
            inline constexpr float SPRITE_HEIGHT = 17.0f;
            inline constexpr float SCALE = 2.0f;
            inline constexpr int FRAMES_PER_ROW = 5;
            inline constexpr int TOTAL_FRAMES = 5;
        } // namespace Player
        namespace Projectile
        {
            namespace Basic
            {
                inline constexpr float SPEED = 800.0f;
                inline constexpr float DAMAGE = 10.0f;
                inline constexpr float LIFETIME = 3.0f;
                inline constexpr float SCALE = 1.0f;
                inline constexpr float FIRE_COOLDOWN = 0.3f;
                inline constexpr float SPRITE_WIDTH = 16.0f;
                inline constexpr float SPRITE_HEIGHT = 8.0f;
            } // namespace Basic

            namespace Supercharged
            {
                inline constexpr float SPEED = 1200.0f;
                inline constexpr float DAMAGE = 25.0f;
                inline constexpr float LIFETIME = 5.0f;
                inline constexpr float SCALE = 1.5f;
                inline constexpr float FIRE_COOLDOWN = 0.2f;
                inline constexpr float CHARGE_TIME = 0.5f;
                inline constexpr float SPRITE_WIDTH = 29.0f;
                inline constexpr float SPRITE_HEIGHT = 24.0f;
                inline constexpr int ANIMATION_FRAMES = 4;
                inline constexpr float ANIMATION_DURATION = 0.15f;
            } // namespace Supercharged
        } // namespace Projectile
        namespace Animation
        {
            inline constexpr float FRAME_DURATION = 0.1f;
        }
        namespace Beam
        {
            inline constexpr float MAX_CHARGE = 1.0f;
            inline constexpr float CHARGE_RATE = 1.0f;
            inline constexpr float BAR_WIDTH = 120.0f;
            inline constexpr float BAR_HEIGHT = 12.0f;
            inline constexpr float BAR_X = 10.0f;
            inline constexpr float BAR_Y = 10.0f;
        } // namespace Beam
        namespace LoadingAnimation
        {
            inline constexpr float SPRITE_WIDTH = 29.0f;
            inline constexpr float SPRITE_HEIGHT = 24.0f;
            inline constexpr int ANIMATION_FRAMES = 4;
            inline constexpr float ANIMATION_DURATION = 0.15f;
            inline constexpr float OFFSET_X = 40.0f;
            inline constexpr float OFFSET_Y = 0.0f;
        } // namespace LoadingAnimation
        namespace Enemy
        {
            namespace Easy
            {
                inline constexpr float HEALTH = 1.0f;
                inline constexpr float DAMAGE = 5.0f;
                inline constexpr float SPEED = 80.0f;
                inline constexpr float SPRITE_WIDTH = 32.0f;
                inline constexpr float SPRITE_HEIGHT = 32.0f;
                inline constexpr float SCALE = 2.0f;
                inline constexpr float SHOOT_COOLDOWN = 2.0f;
                inline constexpr float SPAWN_RATE = 2.0f;
                inline constexpr int ANIMATION_FRAMES = 4;
                inline constexpr float ANIMATION_DURATION = 0.5f;
                inline constexpr int FRAMES_PER_ROW = 4;
            } // namespace Easy
        } // namespace Enemy
        namespace Explosion
        {
            inline constexpr float SPRITE_WIDTH = 32.0f;
            inline constexpr float SPRITE_HEIGHT = 32.0f;
            inline constexpr int ANIMATION_FRAMES = 4;
            inline constexpr float ANIMATION_DURATION = 0.1f;
            inline constexpr int FRAMES_PER_ROW = 4;
            inline constexpr float LIFETIME = 0.4f;
            inline constexpr float SCALE = 2.0f;
        } // namespace Explosion
        namespace Asteroid
        {
            namespace Small
            {
                inline constexpr float HEALTH = 20.0f;
                inline constexpr float SPEED = 80.0f;
                inline constexpr float SPRITE_WIDTH = 18.0f;
                inline constexpr float SPRITE_HEIGHT = 18.0f;
                inline constexpr float SCALE = 2.0f;
                inline constexpr float ROTATION_SPEED = 90.0f;
                inline constexpr float SPAWN_RATE = 1.0f;
                inline constexpr int ANIMATION_FRAMES = 11;
                inline constexpr float ANIMATION_DURATION = 0.5f;
                inline constexpr int FRAMES_PER_ROW = 11;
            } // namespace Small

            namespace Medium
            {
                inline constexpr float HEALTH = 40.0f;
                inline constexpr float SPEED = 60.0f;
                inline constexpr float SPRITE_WIDTH = 32.0f;
                inline constexpr float SPRITE_HEIGHT = 32.0f;
                inline constexpr float SCALE = 1.0f;
                inline constexpr float ROTATION_SPEED = 60.0f;
                inline constexpr float SPAWN_RATE = 2.0f;
            } // namespace Medium

            namespace Large
            {
                inline constexpr float HEALTH = 80.0f;
                inline constexpr float SPEED = 40.0f;
                inline constexpr float SPRITE_WIDTH = 64.0f;
                inline constexpr float SPRITE_HEIGHT = 64.0f;
                inline constexpr float SCALE = 1.5f;
                inline constexpr float ROTATION_SPEED = 30.0f;
                inline constexpr float SPAWN_RATE = 4.0f;
            } // namespace Large
        } // namespace Asteroid
        namespace Hitbox
        {
            inline constexpr float PLAYER_RADIUS = 20.0f;
            inline constexpr float ENEMY_RADIUS = 15.0f;
            inline constexpr float PROJECTILE_BASIC_RADIUS = 5.0f;
            inline constexpr float PROJECTILE_SUPERCHARGED_RADIUS = 8.0f;
            inline constexpr float ASTEROID_SMALL_RADIUS = 25.0f;
            inline constexpr float ASTEROID_MEDIUM_RADIUS = 40.0f;
            inline constexpr float ASTEROID_LARGE_RADIUS = 60.0f;
        } // namespace Hitbox
    } // namespace GameConfig
} // namespace gme
