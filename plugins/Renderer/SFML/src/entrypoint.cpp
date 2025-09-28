#include <memory>

#include "SFMLRenderer/SFMLRenderer.hpp"

extern "C"
{
    std::unique_ptr<eng::IRenderer> entryPoint() { return std::make_unique<eng::SFMLRenderer>(); }
}