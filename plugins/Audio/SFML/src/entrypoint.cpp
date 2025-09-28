#include <memory>

#include "SFMLAudio/SFMLAudio.hpp"

extern "C"
{
    std::unique_ptr<eng::IAudio> entryPoint() { return std::make_unique<eng::SFMLAudio>(); }
}