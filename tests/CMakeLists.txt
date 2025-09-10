option(BUILD_TESTS "Build tests" OFF)
if (NOT BUILD_TESTS)
    return()
endif()

project(r-type_tests
        DESCRIPTION "R-Type unit tests"
        LANGUAGES CXX
)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.hpp)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main)
target_include_directories(${PROJECT_NAME} PRIVATE ${gtest_SOURCE_DIR}/googletest/include ${INCLUDE_DIR})
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
