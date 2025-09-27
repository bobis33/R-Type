add_library(r-type_engine INTERFACE)
add_library(r-type_modules_client INTERFACE)
add_library(r-type_modules_server INTERFACE)

add_subdirectory(Audio)
add_subdirectory(ECS)
add_subdirectory(Engine)
add_subdirectory(Game)
add_subdirectory(Network)
add_subdirectory(Renderer)
add_subdirectory(Utils)

target_link_libraries(r-type_engine INTERFACE
        audio_sfml
        ecs
        engine
        renderer_sfml
        utils
)
target_link_libraries(r-type_modules_client INTERFACE
        engine
        game_rtype_client
        network_asio_client
        utils
)
target_link_libraries(r-type_modules_server INTERFACE
        game_rtype_server
        network_asio_server
        utils
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
        "${CMAKE_CURRENT_SOURCE_DIR}/Network/Asio/Client/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Utils/include"
)
target_include_directories(r-type_modules_server SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Game/R-Type/Server/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Interfaces/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Network/Asio/Server/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/Utils/include"
)
