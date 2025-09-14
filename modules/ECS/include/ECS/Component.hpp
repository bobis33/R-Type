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
            int r;
            int g;
            int b;
            int a;
    };
    struct Font
    {
            std::string id;
            std::string path;
    };
    struct Sprite
    {
            std::string id;
            std::string path;
    };
    struct Text
    {
            std::string id;
            std::string content;
            int fontSize;
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