option(USE_CLANG_TIDY "Enable clang-tidy static analysis" OFF)

if (NOT USE_CLANG_TIDY)
    return()
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy" REQUIRED)

set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=${CMAKE_SOURCE_DIR}/src")
set_source_files_properties(${SOURCES} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY}")
