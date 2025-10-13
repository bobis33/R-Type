project(game_rtype_multi
        DESCRIPTION "R-Type Multiplayer Game Plugin"
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
        "${CMAKE_SOURCE_DIR}/modules/ECS/include"
        "${CMAKE_SOURCE_DIR}/modules/Engine/include"
        "${CMAKE_SOURCE_DIR}/plugins/Game/R-Type/include"
)
target_compile_options(${PROJECT_NAME} PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE game_rtype_common utils ecs engine)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""
)

