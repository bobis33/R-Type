///
/// @file GameConfig.hpp
/// @brief Configuration constants for the multiplayer game
/// @namespace gme
///

#pragma once

namespace utl
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
            inline constexpr float WIDTH = 66.0f;
            inline constexpr float HEIGHT = 17.0f;
            inline constexpr float SCALE = 2.0f;
            inline constexpr int FRAMES_PER_ROW = 5;
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
            inline constexpr float OFFSET_X = 60.0f;
            inline constexpr float OFFSET_Y = 6.0f;
        } // namespace LoadingAnimation
        namespace Stage
        {
            inline constexpr float FLOOR_OFFSET_Y = 16.0f;
            inline constexpr float CEILING_OFFSET_Y = -1.0f;
        } // namespace Stage
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
        namespace Hitbox
        {
            inline constexpr float PLAYER_RADIUS = 20.0f;
            inline constexpr float BOSS_RADIUS = 90.0f;
            inline constexpr float ENEMY_RADIUS = 15.0f;
            inline constexpr float PROJECTILE_BASIC_RADIUS = 5.0f;
            inline constexpr float PROJECTILE_SUPERCHARGED_RADIUS = 8.0f;
        } // namespace Hitbox
        namespace Server
        {
            inline constexpr float SCREEN_WIDTH = 1920.0f;
            inline constexpr float SCREEN_HEIGHT = 1080.0f;
            inline constexpr float WORLD_MARGIN = 200.0f;

            namespace Enemy
            {
                namespace Basic
                {
                    inline constexpr float HEALTH = 50.0f;
                    inline constexpr float DAMAGE = 10.0f;
                    inline constexpr float SPEED = 200.0f;
                    inline constexpr float SHOOT_COOLDOWN = 2.0f;
                    inline constexpr float HITBOX_RADIUS = 15.0f;
                } // namespace Basic

                namespace Advanced
                {
                    inline constexpr float HEALTH = 100.0f;
                    inline constexpr float DAMAGE = 15.0f;
                    inline constexpr float SPEED = 150.0f;
                    inline constexpr float SHOOT_COOLDOWN = 1.5f;
                    inline constexpr float HITBOX_RADIUS = 15.0f;
                    inline constexpr float SINE_FREQUENCY = 1.5f;
                    inline constexpr float SINE_AMPLITUDE = 100.0f;
                } // namespace Advanced

                namespace Boss
                {
                    inline constexpr float HEALTH = 1000.0f;
                    inline constexpr float DAMAGE = 50.0f;
                    inline constexpr float HITBOX_RADIUS = 50.0f;
                    inline constexpr float RAM_DAMAGE = 20.0f;
                    inline constexpr float PHASE_2_THRESHOLD = 0.66f;
                    inline constexpr float PHASE_3_THRESHOLD = 0.33f;
                    inline constexpr float PHASE1_SPEED = 30.0f;
                    inline constexpr float PHASE1_COOLDOWN = 1.5f;
                    inline constexpr float PHASE1_SINE_FREQUENCY = 0.5f;
                    inline constexpr float PHASE1_SINE_AMPLITUDE = 150.0f;
                    inline constexpr float PHASE2_COOLDOWN = 1.0f;
                    inline constexpr float PHASE3_SPEED = 50.0f;
                    inline constexpr float PHASE3_COOLDOWN = 0.5f;
                    inline constexpr float PHASE3_AGGRESSION = 80.0f;
                    inline constexpr float SPREAD_INTERVAL = 1.0f;
                    inline constexpr float SPREAD_ANGLE_DEGREES = 15.0f;
                    inline constexpr float SPREAD_SPEED = 300.0f;
                } // namespace Boss

                namespace AI
                {
                    inline constexpr float SHOOT_CHANCE = 0.3f;
                    inline constexpr float MAX_SHOOT_DISTANCE = 1500.0f;
                    inline constexpr float AGGRESSIVE_ACTIVATION_DISTANCE = 400.0f;
                    inline constexpr float ZIGZAG_PERIOD = 1.5f;
                    inline constexpr float ZIGZAG_SPEED_X = 150.0f;
                    inline constexpr float ZIGZAG_SPEED_Y = 100.0f;
                } // namespace AI
            } // namespace Enemy

            namespace Projectile
            {
                inline constexpr float ENEMY_SPEED = 500.0f;
                inline constexpr float ENEMY_DAMAGE = 15.0f;
                inline constexpr float ENEMY_LIFETIME = 10.0f;
            } // namespace Projectile

            namespace Network
            {
                inline constexpr float BROADCAST_RATE = 60.0f;
                inline constexpr float BROADCAST_INTERVAL = 1.0f / BROADCAST_RATE;
            } // namespace Network
        } // namespace Server
    } // namespace GameConfig
} // namespace utl
