# Renderer plugin

## How to add a new renderer plugin

First you will have to create a directory corresponding on ur implementation
inside the `plugins/Renderer` directory. For example, if you are implementing
a renderer plugin for the "SDL" graphic library, you would create a directory
named `SDL`.
Inside this directory, you will need to create at least:
- CMakeLists.txt
```cmake
project(renderer_SDL
        DESCRIPTION "Renderer SDL Plugin"
        LANGUAGES C CXX
)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIR}
)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE r-type_modules_client)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
)
```
> If needed, link to your dependency libraries using `target_link_libraries`.

- Include dir with your header files (e.g., `include/SDL/SDL.hpp`). Don't forget to
  make your class inherit from `IRenderer`.
```c++
///
/// @file SDL.hpp
/// @brief SDL renderer class declaration
/// @namespace eng
///

#pragma once

#include "Interfaces/IRenderer.hpp"

namespace eng
{

    ///
    /// @class SDL
    /// @brief Class for rendering with SDL
    /// @namespace eng
    ///
    class SDL final : public IRenderer
    {
        public:
            SDL() = default;
            ~SDL() override = default;

            SDL(const SDL &) = delete;
            SDL &operator=(const SDL &) = delete;
            SDL(SDL &&) = delete;
            SDL &operator=(SDL &&) = delete;

            [[nodiscard]] const std::string getName() const override { return "Renderer_SFML"; }
            [[nodiscard]] utl::PluginType getType() const override { return utl::PluginType::RENDERER; }

            void createWindow(const std::string &title, unsigned int height, unsigned int width,
                              unsigned int frameLimit, bool fullscreen) override;
            bool windowIsOpen() const override;
            void closeWindow() override;
            void clearWindow(Color color) override;
            void displayWindow() override;
            WindowSize getWindowSize() override;
            void setWindowIcon(const std::string &path) override;

            bool pollEvent(Event &event) override;
            void setFrameLimit(unsigned int frameLimit) override;

            void createFont(const std::string &name, const std::string &path) override;
            void createText(Text text) override;
            void setTextContent(const std::string &name, const std::string &content) override;
            void setTextPosition(const std::string &name, float x, float y) override;
            void setTextColor(const std::string &name, Color color) override;

            void drawText(const std::string &name) override;
            void createTexture(const std::string &name, const std::string &path) override;
            void createSprite(const std::string &name, const std::string &textureName, float x, float y, float scale_x,
                              float scale_y, int fx, int fy, int fnx, int fny) override;
            void setSpritePosition(const std::string &name, float x, float y) override;
            void setSpriteTexture(const std::string &name, const std::string &path) override;
            void setSpriteFrame(const std::string &name, int fx, int fy, int fnx, int fny) override;
            void setSpriteScale(const std::string &name, int x, int y) override;
            void setSpriteColor(const std::string &name, Color color) override;
            void drawSprite(const std::string &name) override;
            void setSpriteRotation(const std::string &name, float rotation) override;
            void rotateSprite(const std::string &name, float rotation) override;
            void setSpriteOrigin(const std::string &name) override;

            void createCircleShape(CircleShape circleShape) override;
            void setCircleShapePosition(const std::string &name, float x, float y) override;
            void drawCircleShape(const std::string &name) override;

            void drawPoint(float x, float y, Color color) override;
            void renderGui(const WindowSize &windowSize) override;

    }; // class SDL

} // namespace eng
```
- Source dir with your implementation files (e.g., `src/SDL.cpp`), and your entrypoint:

```c++
#include <memory>

#include "SFMLAudio/SFMLAudio.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::IRenderer *entryPoint() { return std::make_unique<eng::SDL>().release(); }
}
```

Then your setup is done! You just need to add in the plugin/Renderer `CMakeLists.txt` the directory of your plugin:

```cmake
add_subdirectory(SDL)
```