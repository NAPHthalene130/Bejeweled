# FindGLFW3.cmake
# Locates the GLFW3 library
#
# This module defines:
# GLFW3_FOUND - true if GLFW3 was found
# GLFW3_INCLUDE_DIR - the GLFW3 include directory
# GLFW3_LIBRARY - the GLFW3 library
#
# Usage:
# find_package(GLFW3 REQUIRED)
# target_link_libraries(target PRIVATE GLFW3::GLFW3)

# If the user has provided a GLFW_DIR via environment, use it as a hint
set(_GLFW3_HINTS "$ENV{GLFW_DIR}")

find_path(GLFW3_INCLUDE_DIR
    NAMES GLFW/glfw3.h
    HINTS ${_GLFW3_HINTS}
    PATH_SUFFIXES include
)

find_library(GLFW3_LIBRARY
    NAMES glfw3 glfw3dll
    HINTS ${_GLFW3_HINTS}
    PATH_SUFFIXES lib-mingw-w64 lib-vc2022 lib lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW3
    REQUIRED_VARS GLFW3_LIBRARY GLFW3_INCLUDE_DIR
)

if(GLFW3_FOUND AND NOT TARGET GLFW3::GLFW3)
    add_library(GLFW3::GLFW3 UNKNOWN IMPORTED)
    set_target_properties(GLFW3::GLFW3 PROPERTIES
        IMPORTED_LOCATION "${GLFW3_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW3_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GLFW3_INCLUDE_DIR GLFW3_LIBRARY)
