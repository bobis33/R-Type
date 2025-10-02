#include <memory>

#include "SFMLAudio/SFMLAudio.hpp"

extern "C"
{
    eng::IAudio *entryPoint() { return std::make_unique<eng::SFMLAudio>().release(); }
}