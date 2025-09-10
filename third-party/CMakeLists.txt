add_library(r-type_third-party INTERFACE)
#======================================= GoogleTest =======================================#
if (BUILD_TESTS)
    add_subdirectory(googletest)
endif()
