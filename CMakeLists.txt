cmake_minimum_required(VERSION 4.0.0)
project(r-type
        VERSION 0.0.0
        DESCRIPTION "RType main"
        LANGUAGES C CXX
)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/lib")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
add_compile_definitions(PLUGINS_DIR="${LIBRARY_OUTPUT_PATH}")
##include(MakeVersion)
include(MakeDoc)
include(ClangTidy)
include(ClangFormat)
add_subdirectory(third-party)
add_subdirectory(modules)
add_subdirectory(plugins)
## include(MakeShaders) ## OPG / VLK
add_subdirectory(tests)
add_subdirectory(client)
add_subdirectory(server)