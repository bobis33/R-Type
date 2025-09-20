///
/// @file Component.hpp
/// @brief This file contains the component definitions
/// @namespace ecs
///

#pragma once

#include <string>

namespace ecs
{
    struct Audio
    {
            std::string id;
            std::string path;
            float volume;
            bool loop;
    };
    struct Color
    {
            std::string id;
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
    };
    struct Font
    {
            std::string id;
            std::string path;
    };
    struct Point
    {
            std::string id;
            float x, y;
            Color color; // TODO(bobis33): to remove
    };
    struct Rect
    {
            std::string id;
            float pos_x, pos_y;
            int size_x, size_y;
    };
    struct Scale
    {
            std::string id;
            float x, y;
    };
    struct Text
    {
            std::string id;
            std::string content;
            unsigned int fontSize;
    };
    struct Texture
    {
            std::string id;
            std::string path;
    };
    struct Transform
    {
            std::string id;
            float x, y;
            float rotation;
    };
    struct Velocity
    {
            std::string id;
            float x, y;
    };
} // namespace ecs