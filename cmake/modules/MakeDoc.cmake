option(BUILD_DOC "Build documentation" OFF)

if (NOT BUILD_DOC)
    return()
endif()

find_package(Doxygen REQUIRED)

set(DOXYGEN_DIR "${CMAKE_SOURCE_DIR}/documentation/.doxygen")
set(DOXYFILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
configure_file(${DOXYGEN_DIR}/Doxyfile ${DOXYFILE_OUT} @ONLY)
doxygen_add_docs(doxygen
        "${CMAKE_SOURCE_DIR}/client/include"
        "${CMAKE_SOURCE_DIR}/client/src"
        "${CMAKE_SOURCE_DIR}/server/include"
        "${CMAKE_SOURCE_DIR}/server/src"
        "${CMAKE_SOURCE_DIR}/modules/Utils/include"
        "${CMAKE_SOURCE_DIR}/modules/Utils/src"
)
add_custom_command(TARGET doxygen POST_BUILD
        WORKING_DIRECTORY ${DOXYGEN_DIR}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        VERBATIM
)
add_custom_command(TARGET doxygen POST_BUILD
        WORKING_DIRECTORY "${DOXYGEN_DIR}/latex"
        COMMAND make > /dev/null
        COMMAND ${CMAKE_COMMAND} -E copy refman.pdf "${CMAKE_SOURCE_DIR}/documentation/R-Type.pdf"
        BYPRODUCTS "${CMAKE_SOURCE_DIR}/documentation/R-Type.pdf"
        VERBATIM
)
