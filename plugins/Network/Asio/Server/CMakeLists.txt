project(network_asio_server
        DESCRIPTION "Network Asio Server Plugin"
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
        "${CMAKE_SOURCE_DIR}/third-party/asio/asio/include"
        "${CMAKE_SOURCE_DIR}/plugins/Network/Protocol/include"
)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00>
)
target_link_libraries(${PROJECT_NAME} PRIVATE utils)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
)
