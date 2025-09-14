///
/// @file Entity.hpp
/// @brief This file contains the entity definitions
/// @namespace ecs
///

#pragma once

#include <cstdint>

namespace ecs
{
    using Entity = std::uint32_t;
    constexpr Entity INVALID_ENTITY = 0;
} // namespace ecs