add_library(r-type_modules INTERFACE)

add_subdirectory(Utils)

target_link_libraries(r-type_modules INTERFACE
        utils
)
target_include_directories(r-type_modules SYSTEM INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Utils/include"
)
