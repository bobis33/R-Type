# Game plugin

## How to add a new game plugin

First you will have to Game a directory corresponding on ur implementation
inside the `plugins/Network` directory. For example, if you are implementing
a game plugin for the "Snake" game, you would create a directory
named `Snake`.
Inside this directory, you will need to create at least:
- `Client` and `Server` directory
- CMakeLists.txt
```cmake
add_subdirectory(Client)
add_subdirectory(Server)
```

### Client
- Client/CMakeLists.txt
```cmake
project(game_snakeClient
        DESCRIPTION "Game Snake client Plugin"
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

- Include dir with your header files (e.g., `include/UnixSocketClient/UnixSocketClient.hpp`). Don't forget to
  make your class inherit from `INetworkClient`.
```c++
///
/// @file SnakeClient.hpp
/// @brief snake client game class declaration
/// @namespace eng
///

#pragma once

#include "Interfaces/IGameClient.hpp"

namespace eng
{

    ///
    /// @class SnakeClient
    /// @brief Class for snake game client
    /// @namespace eng
    ///
    class SnakeClient final : public IGameClient
    {
        public:
            SnakeClient() = default;
            ~SnakeClient() override = default;

            SnakeClient(const SnakeClient &) = delete;
            SnakeClient &operator=(const SnakeClient &) = delete;
            SnakeClient(SnakeClient &&) = delete;
            SnakeClient &operator=(SnakeClient &&) = delete;

    }; // class SnakeClient

} // namespace eng
```
- Source dir with your implementation files (e.g., `src/SnakeClient.cpp`), and your entrypoint:

```c++
#include <memory>

#include "SnakeClient/SnakeClient.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::IGameClient *entryPoint() { return std::make_unique<eng::SnakeClient>().release(); }
}
```

### Server

- Server/CMakeLists.txt
```cmake
project(game_SnakeServer
        DESCRIPTION "Game snake server Plugin"
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

- Include dir with your header files (e.g., `include/SnakeServer/SnakeServer.hpp`). Don't forget to
  make your class inherit from `IGameServer`.
```c++
///
/// @file INetworkServer.hpp
/// @brief Snake Server network class declaration
/// @namespace eng
///

#pragma once

#include "Interfaces/IGameServer.hpp"

namespace eng
{

    ///
    /// @class SnakeServer
    /// @brief Class for snake game server
    /// @namespace eng
    ///
    class SnakeServer final : public IGameServer
    {
        public:
            SnakeServer() = default;
            ~SnakeServer() override = default;

            SnakeServer(const SnakeServer &) = delete;
            SnakeServer &operator=(const SnakeServer &) = delete;
            SnakeServer(SnakeServer &&) = delete;
            SnakeServer &operator=(SnakeServer &&) = delete;

    }; // class SnakeServer

} // namespace eng
```
- Source dir with your implementation files (e.g., `src/SnakeServer.cpp`), and your entrypoint:

```c++
#include <memory>

#include "SnakeServer/SnakeServer.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::IGameServer *entryPoint() { return std::make_unique<eng::SnakeServer>().release(); }
}
```