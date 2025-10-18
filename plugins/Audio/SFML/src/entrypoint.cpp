#include <memory>

#include "SFMLAudio/SFMLAudio.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::IAudio *entryPoint() { return std::make_unique<eng::SFMLAudio>().release(); }
}