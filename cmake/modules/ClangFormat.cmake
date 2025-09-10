option(USE_CLANG_FORMAT "Enable clang-format" OFF)

if (NOT USE_CLANG_FORMAT)
    return()
endif()

find_program(CLANG_FORMAT_EXE NAMES clang-format REQUIRED)

file(GLOB_RECURSE ALL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/include/*.hpp"
)
add_custom_target(clang-format
        COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting sources with clang-format"
)
add_custom_target(clang-format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror -style=file ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking source formatting"
)