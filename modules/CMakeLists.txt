add_library(r-type_engine INTERFACE)
add_library(r-type_modules_client INTERFACE)
add_library(r-type_modules_server INTERFACE)

add_subdirectory(ECS)
add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(Utils)

target_link_libraries(r-type_engine INTERFACE
        ecs
        engine
        utils
)
target_link_libraries(r-type_modules_client INTERFACE
        engine
        game_rtype_client
        utils
)
target_link_libraries(r-type_modules_server INTERFACE
        game_rtype_server
        utils
)

target_include_directories(r-type_engine SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/ECS/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Utils/include"
)
target_include_directories(r-type_modules_client SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Engine/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Game/R-Type/Client/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Utils/include"
)
target_include_directories(r-type_modules_server SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Game/R-Type/Server/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Utils/include"
)
