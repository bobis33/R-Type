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
} // namespace ecs