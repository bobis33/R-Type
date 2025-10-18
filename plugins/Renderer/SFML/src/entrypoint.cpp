#include <memory>

#include "SFMLRenderer/SFMLRenderer.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::IRenderer *entryPoint() { return std::make_unique<eng::SFMLRenderer>().release(); }
}