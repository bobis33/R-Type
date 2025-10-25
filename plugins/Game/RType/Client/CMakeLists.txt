add_library(r-type_game_client_shared INTERFACE)

target_link_libraries(r-type_game_client_shared INTERFACE)

target_include_directories(r-type_game_client_shared SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Shared/include"
)

add_subdirectory(Multi)
add_subdirectory(Solo)