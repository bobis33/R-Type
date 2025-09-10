cmake_minimum_required(VERSION 4.0.0)
project(r-type_client
        VERSION 0.0.0
        DESCRIPTION ""
        LANGUAGES CXX
)
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
include(MakeVersion)
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
add_dependencies(${PROJECT_NAME} r-type_third-party r-type_modules)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE r-type_modules)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
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
else ()
    if (MSVC)
        set(WARNING_FLAGS /W3)
    else()
        set(WARNING_FLAGS -Wno-error)
    endif()
endif()
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} /O2)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} -O3)
endif ()
##add_compile_definitions(PLUGINS_DIR="${LIBRARY_OUTPUT_PATH}")
