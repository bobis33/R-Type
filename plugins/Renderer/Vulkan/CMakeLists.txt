project(cae-vulkan
        DESCRIPTION "CAE Vulkan Renderer Plugin"
        LANGUAGES C CXX
)

find_package(Vulkan REQUIRED)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

file(GLOB_RECURSE SOURCE "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
)

add_library(${PROJECT_NAME} SHARED
        ${SOURCE}
        ${HEADERS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${PROJECT_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/include"
)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
## target_link_libraries(${PROJECT_NAME} PRIVATE cae-modules)