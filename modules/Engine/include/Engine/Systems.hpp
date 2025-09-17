///
/// @file Systems.hpp
/// @brief This file contains the system definitions
/// @namespace eng
///

#pragma once

#include "ECS/Registry.hpp"

namespace eng
{

    class ISystem
    {
        public:
            virtual ~ISystem() = default;
            virtual void update(ecs::Registry &registry, float dt) = 0;
            virtual bool isEnable() = 0;
            virtual void setEnable(bool enable) = 0;
    };

} // namespace eng