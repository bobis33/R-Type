///
/// @file Component.hpp
/// @brief This file contains the component definitions
/// @namespace ecs
///

#pragma once

#include <string>

namespace ecs
{
    struct IComponent
    {
            std::string id;
    };
    struct Audio final : IComponent
    {
            std::string path;
            float volume;
            bool loop;
            bool play;
    };
    struct Color final : IComponent
    {
            unsigned char r{};
            unsigned char g{};
            unsigned char b{};
            unsigned char a{};
    };
    struct Font final : IComponent
    {
            std::string path;
    };
    struct Mob final : IComponent
    {
            // bool is_alive{};
    };
    struct Player final : IComponent
    {
            bool is_cli{};
            // bool is_alive{};
    };
    struct Pixel final : IComponent
    {
    };
    struct Rect final : IComponent
    { // TODO(bobis33): remove, only used for texture actually
            float pos_x{}, pos_y{};
            int size_x{}, size_y{};
    };
    struct Scale final : IComponent
    {
            float x{}, y{};
    };
    struct Text final : IComponent
    {
            std::string content;
            unsigned int font_size;
    };
    struct Texture final : IComponent
    {
            std::string path;
            // float rect_pos_x{}, rect_pos_y{};
            // int rect_size_x{}, rect_size_y{};
    };
    struct Transform final : IComponent
    {
            float x{}, y{};
            float rotation{};
    };
    struct Velocity final : IComponent
    {
            float x{}, y{};
    };
    struct Animation final : IComponent
    {
            int current_frame{};
            int total_frames{};
            float frame_duration{};
            float current_time{};
            int frame_width{};
            int frame_height{};
            int frames_per_row{};
    };
    struct Projectile final : IComponent
    {
            enum Type
            {
                BASIC,
                SUPERCHARGED
            };
            Type type;
            float damage;
            float lifetime;
            float current_lifetime;
    };

    struct BeamCharge final : IComponent
    {
            float current_charge;
            float max_charge;
    };

    struct LoadingAnimation final : IComponent
    {
            int current_frame;
            int total_frames;
            float frame_duration;
            float current_time;
            float frame_width;
            float frame_height;
            int frames_per_row;
    };

    struct Enemy final : IComponent
    {
            float health;
            float max_health;
            float damage;
            float speed;
            float last_shot_time;
            float shoot_cooldown;
    };

    struct Asteroid final : IComponent
    {
            enum Size
            {
                SMALL,
                MEDIUM,
                LARGE
            };
            Size size;
            float rotation_speed;
            float health;
    };

    struct Explosion final : IComponent
    {
            int current_frame;
            int total_frames;
            float frame_duration;
            float current_time;
            float frame_width;
            float frame_height;
            int frames_per_row;
            float lifetime;
            float current_lifetime;
    };

    struct Hitbox final : IComponent
    {
            float radius;
    };

    struct KeyboardInput final : IComponent
    {
            bool space_pressed{};
            bool up_pressed{};
            bool down_pressed{};
            bool left_pressed{};
            bool right_pressed{};
    };

} // namespace ecs