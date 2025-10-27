add_library(r-type_third-party INTERFACE)
#==================================== GoogleTest ====================================#
if (BUILD_TESTS)
    add_subdirectory(googletest)
endif()

#==================================== SFML ====================================#
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)

set(SFML_USE_SYSTEM_DEPS FALSE CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS FALSE CACHE BOOL "" FORCE)
add_subdirectory(SFML)

#==================================== ImGui-SFML ====================================#
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui" CACHE PATH "" FORCE)
set(IMGUI_SFML_USE_STATIC_LIBS TRUE CACHE BOOL "" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" OFF)
add_subdirectory(imgui-sfml)