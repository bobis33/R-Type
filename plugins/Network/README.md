# Network plugin

## How to add a new network plugin

First you will have to create a directory corresponding on ur implementation
inside the `plugins/Network` directory. For example, if you are implementing
a network plugin for the "Unix socket" library, you would create a directory
named `UnixSocket`.
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
project(network_UnixSocketClient
        DESCRIPTION "Network Unix Socket client Plugin"
        LANGUAGES C CXX
)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE r-type_modules_client)
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
/// @file UnixSocketClient.hpp
/// @brief Unix Socket client network class declaration
/// @namespace eng
///

#pragma once

#include "Interfaces/INetworkClient.hpp"

namespace eng
{

    ///
    /// @class UnixSocketClient
    /// @brief Class for network with unix socket
    /// @namespace eng
    ///
    class UnixSocketClient final : public INetworkCLient
    {
        public:
            UnixSocketClient() = default;
            ~UnixSocketClient() override = default;

            UnixSocketClient(const UnixSocketClient &) = delete;
            UnixSocketClient &operator=(const UnixSocketClient &) = delete;
            UnixSocketClient(UnixSocketClient &&) = delete;
            UnixSocketClient &operator=(UnixSocketClient &&) = delete;

            [[nodiscard]] bool isConnected() const override;
            [[nodiscard]] ConnectionState getConnectionState() const override;
            [[nodiscard]] std::uint32_t getSessionId() const override;
            [[nodiscard]] std::uint16_t getServerTickRate() const override;
            [[nodiscard]] std::uint32_t getLatency() const override;

            void setPlayerName(const std::string &playerName) override;
            void setClientCapabilities(std::uint32_t caps) override;

    }; // class UnixSocketClient

} // namespace eng
```
- Source dir with your implementation files (e.g., `src/UnixSocketClient.cpp`), and your entrypoint:

```c++
#include <memory>

#include "UnixSocketClient/UnixSocketClient.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::INetworkClient *entryPoint() { return std::make_unique<eng::UnixSocketClient>().release(); }
}
```

### Server

- Server/CMakeLists.txt
```cmake
project(network_UnixSocketServer
        DESCRIPTION "Network Unix Socket server Plugin"
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
target_link_libraries(${PROJECT_NAME} PRIVATE r-type_modules_server)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
)
```
> If needed, link to your dependency libraries using `target_link_libraries`.

- Include dir with your header files (e.g., `include/UnixSocketServer/UnixSocketServer.hpp`). Don't forget to
  make your class inherit from `INetworkServer`.
```c++
///
/// @file INetworkServer.hpp
/// @brief Unix Socket server network class declaration
/// @namespace eng
///

#pragma once

#include "Interfaces/INetworkServer.hpp"

namespace eng
{

    ///
    /// @class UnixSocketServer
    /// @brief Class for network with unix socket
    /// @namespace eng
    ///
    class UnixSocketServer final : public INetworkServer
    {
        public:
            UnixSocketServer() = default;
            ~UnixSocketServer() override = default;

            UnixSocketServer(const UnixSocketServer &) = delete;
            UnixSocketServer &operator=(const UnixSocketServer &) = delete;
            UnixSocketServer(UnixSocketServer &&) = delete;
            UnixSocketServer &operator=(UnixSocketServer &&) = delete;

            void init(const std::string &host, std::uint16_t port) override;
            void start() override;
            void stop() override;
            void update() override;
            void sendToClient(std::uint32_t sessionId, const std::vector<std::uint8_t> &data,
                              bool reliable = false) override;
            void sendToAllClients(const std::vector<std::uint8_t> &data, bool reliable = false) override;
            void disconnectClient(std::uint32_t sessionId) override;

            [[nodiscard]] std::size_t getClientCount() const override;
            [[nodiscard]] std::vector<std::uint32_t> getConnectedSessions() const override;
            [[nodiscard]] bool isRunning() const override;

            void setTickRate(std::uint16_t tickRate) override;
            void setServerCapabilities(std::uint32_t caps) override;


    }; // class UnixSocketServer

} // namespace eng
```
- Source dir with your implementation files (e.g., `src/UnixSocketServer.cpp`), and your entrypoint:

```c++
#include <memory>

#include "UnixSocketServer/UnixSocketServer.hpp"

extern "C"
{
    PLUGIN_EXPORT eng::INetworkClient *entryPoint() { return std::make_unique<eng::UnixSocketServer>().release(); }
}
```