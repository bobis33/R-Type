add_library(r-type_engine INTERFACE)
add_library(r-type_modules_client INTERFACE)
add_library(r-type_modules_server INTERFACE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Utils)
add_subdirectory(ECS)
add_subdirectory(Audio)
add_subdirectory(Renderer)
add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(Network)

target_link_libraries(r-type_engine INTERFACE
        utils
        ecs
        renderer_sfml
        audio_sfml
        engine
)
target_link_libraries(r-type_modules_client INTERFACE
        utils
        game_rtype_client
        engine
        network_rtype_client
)
target_link_libraries(r-type_modules_server INTERFACE
        utils
        game_rtype_server
        network_rtype_server
)

target_include_directories(r-type_engine SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Audio/SFMLAudio/include"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/Network/include"
)
