cmake_minimum_required(VERSION 4.0.0)
project(r-type
        VERSION 0.0.0
        DESCRIPTION "R-Type main"
        LANGUAGES C CXX
)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
execute_process(COMMAND ln -s ${PROJECT_SOURCE_DIR}/cmake-build-release/compile_commands.json ${PROJECT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
if (NOT WIN32)
    set(WARNING_FLAGS
            -Wall
            -Wextra
            -Wdeprecated-copy
            -Wmisleading-indentation
            -Wnull-dereference
            -Woverloaded-virtual
            -Wpedantic
            -Wshadow
            -Wsign-conversion
            -Wnon-virtual-dtor
            -Wunused
            -Wcast-align
            -Wno-padded
            -Wconversion
            -Wformat
            -Winit-self
            -Wmissing-include-dirs
            -Wold-style-cast
            -Wredundant-decls
            -Wswitch-default
            -Wundef
    )
elseif (WIN32 AND MSVC)
    set(WARNING_FLAGS /W3)
else()
    set(WARNING_FLAGS -Wno-error)
endif()
add_compile_definitions(PLUGINS_DIR="${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
include(MakeDoc)
include(ClangTidy)
include(ClangFormat)
add_subdirectory(third-party)
add_subdirectory(modules)
add_subdirectory(plugins)
## include(MakeShaders) ## OPGL / VLK
add_subdirectory(tests)
add_subdirectory(client)
add_subdirectory(server)