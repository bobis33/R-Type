project(network_asio_client
        DESCRIPTION "Network client library"
        LANGUAGES CXX
)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE HEADERS "${INCLUDE_DIRS}/*.hpp")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/modules/Interfaces/include
        ${CMAKE_SOURCE_DIR}/third-party/asio/asio/include
)

target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
