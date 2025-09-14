find_package(Git REQUIRED)

execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT GIT_COMMIT_HASH)
    set(GIT_COMMIT_HASH "unknown")
    message(WARNING "Git commit hash not found")
endif()
execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT GIT_TAG)
    set(GIT_TAG "v0.0.0-unknown")
    message(WARNING "Git tag not found")
endif()
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"
        SEMVER_VERSION
        ${GIT_TAG}
)
configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/config/Version.hpp.in"
        "${PATH_VERSION}"
)
