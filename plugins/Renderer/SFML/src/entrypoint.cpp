#include <memory>

#include "SFMLRenderer/SFMLRenderer.hpp"

extern "C"
{
    eng::IRenderer *entryPoint() { return std::make_unique<eng::SFMLRenderer>().release(); }
}