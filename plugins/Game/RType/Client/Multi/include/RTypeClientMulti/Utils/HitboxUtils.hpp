#pragma once

#include <tuple>

namespace gme
{
    namespace Utils
    {
        inline std::tuple<float, float> calculateHitboxOffsets(float spriteWidth, float spriteHeight, float scale)
        {
            const float halfWidth = (spriteWidth * scale) / 2.0f;
            const float halfHeight = (spriteHeight * scale) / 2.0f;
            return {halfWidth, halfHeight};
        }
    } // namespace Utils
} // namespace gme
