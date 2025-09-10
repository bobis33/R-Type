#include <memory>

#include "VULKN/VULKN.hpp"

extern "C"
{
    std::unique_ptr<cae::IRenderer> entryPoint()
    {
        return std::make_unique<cae::VULKN>();
    }
}