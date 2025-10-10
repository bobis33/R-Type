cmake_minimum_required(VERSION 4.0.0)
project(r-type_server
        VERSION 0.0.0
        DESCRIPTION "R-TYPE Server"
        LANGUAGES C CXX
)
set(PATH_VERSION "${PROJECT_SOURCE_DIR}/include/Server/Generated/Version.hpp")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/R-Type/*.hpp")
include(MakeVersion)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
add_dependencies(${PROJECT_NAME}
        r-type_third-party
        r-type_modules_server
)
target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIR}
        "${CMAKE_SOURCE_DIR}/third-party/json/single_include"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        r-type_modules_server
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
if (MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} /RTC1)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} /O2)
    endif()
else ()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} -g)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS} -O3)
    endif()
endif ()
