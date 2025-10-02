add_library(r-type_third-party INTERFACE)
#==================================== GoogleTest ====================================#
if (BUILD_TESTS)
    add_subdirectory(googletest)
endif()

#==================================== SFML ====================================#
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)

set(SFML_USE_SYSTEM_DEPS FALSE CACHE BOOL "" FORCE)
set(SFML_STATIC_LIBRARIES TRUE CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)
add_subdirectory(SFML)