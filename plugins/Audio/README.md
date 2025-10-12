# Audio plugin

## How to add a new audio plugin

First you will have to create a directory corresponding on ur implementation
inside the `plugins/Audio` directory. For example, if you are implementing
an audio plugin for the "SuperSound" audio library, you would create a directory
named `SuperSound`.
Inside this directory, you will need to create at least:
- CMakeLists.txt
```cmake
project(audio_SuperSound
        DESCRIPTION "Audio SuperSound Plugin"
        LANGUAGES C CXX
)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIR}
        "${CMAKE_SOURCE_DIR}/modules/Interfaces/include"
        "${CMAKE_SOURCE_DIR}/modules/Utils/include"
)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE utils)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
)
```
> If needed, link to your dependency libraries using `target_link_libraries`.

- Include dir with your header files (e.g., `include/SuperSound/AudioSuperSound.hpp`). Dont forget to
make your class inherit from `IAudio`.
```c++
///
/// @file SuperSoundAudio.hpp
/// @brief SuperSoundAudio class declaration
/// @namespace eng
///

#pragma once

#include "Interfaces/IAudio.hpp"

namespace eng
{

    ///
    /// @class SuperSoundAudio
    /// @brief Class for audio management
    /// @namespace eng
    ///
    class SuperSoundAudio final : public IAudio
    {
        public:
            SuperSoundAudio() = default;
            ~SuperSoundAudio() override = default;

            SuperSoundAudio(const SuperSoundAudio &) = delete;
            SuperSoundAudio &operator=(const SuperSoundAudio &) = delete;
            SuperSoundAudio(SuperSoundAudio &&) = delete;
            SuperSoundAudio &operator=(SuperSoundAudio &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Audio_SuperSound"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::AUDIO; }

            void createAudio(const std::string &path, float volume, bool loop, const std::string &name) override;
            void playAudio(const std::string &name) override;
            void setVolume(const std::string &name, float volume) override;
            void setLoop(const std::string &name, bool loop) override;
            void stopAudio(const std::string &name) override;
            Status isPlaying(const std::string &name) override;

        private:

    }; // class SuperSoundAudio

} // namespace eng
```
- Source dir with your implementation files (e.g., `src/SuperSoundAudio.cpp`), and your entrypoint:

```c++
#include <memory>

#include "SFMLAudio/SFMLAudio.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::IAudio *entryPoint() { return std::make_unique<eng::SuperSoundAudio>().release(); }
}
```

Then your setup is done! You just need to add in the plugin/audio `CMakeLists.txt` the directory of your plugin:

```cmake
add_subdirectory(SuperSound)
```