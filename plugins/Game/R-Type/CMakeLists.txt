project(game_rtype
        DESCRIPTION "R-Type Game Plugin Common Library"
        LANGUAGES C CXX
)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# Source files for the common library (excluding old entrypoint and RTypeGame)
set(COMMON_SOURCES
    "${SRC_DIR}/AGamePlugin.cpp"
    "${SRC_DIR}/Scenes/Menu.cpp"
    "${SRC_DIR}/Scenes/GameSolo.cpp"
    "${SRC_DIR}/ProjectileManager.cpp"
    "${SRC_DIR}/Systems/Weapon.cpp"
)

file(GLOB_RECURSE HEADERS "${INCLUDE_DIR}/*.hpp")

# Create a static library for common code
add_library(${PROJECT_NAME}_common STATIC ${COMMON_SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME}_common PUBLIC
        ${INCLUDE_DIR}
        "${CMAKE_SOURCE_DIR}/modules/Interfaces/include"
        "${CMAKE_SOURCE_DIR}/modules/Utils/include"
        "${CMAKE_SOURCE_DIR}/modules/ECS/include"
        "${CMAKE_SOURCE_DIR}/modules/Engine/include"
)
target_compile_options(${PROJECT_NAME}_common PRIVATE ${WARNING_FLAGS})
target_link_libraries(${PROJECT_NAME}_common PUBLIC utils ecs engine)
target_compile_features(${PROJECT_NAME}_common PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME}_common PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

