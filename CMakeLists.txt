cmake_minimum_required(VERSION 3.19)

# ==============================================================================
# Environment Configuration (Pre-Project)
# ==============================================================================
# Read .vscode/.env file to set environment variables for dependencies
# We do this BEFORE project() so we can set compilers if needed.
if(EXISTS "${CMAKE_SOURCE_DIR}/.vscode/.env")
    message(STATUS "Loading configuration from .vscode/.env file...")
    file(STRINGS "${CMAKE_SOURCE_DIR}/.vscode/.env" ConfigContents)
    foreach(line ${ConfigContents})
        # Ignore comments and empty lines
        if(line MATCHES "^[ \t]*[^#=]+=.+$")
            string(REGEX REPLACE "^[ \t]*([^=]+)=(.*)$" "\\1;\\2" key_value "${line}")
            list(GET key_value 0 key)
            list(GET key_value 1 value)
            string(STRIP "${key}" key)
            string(STRIP "${value}" value)
            set(ENV{${key}} "${value}")
            message(STATUS "  Set ${key}=${value}")
        endif()
    endforeach()
else()
    message(WARNING ".vscode/.env file not found! Please copy .vscode/.env.template to .vscode/.env and configure your paths.")
endif()

# Set Compiler from MINGW_PATH if provided and not already set
if(NOT DEFINED CMAKE_CXX_COMPILER AND DEFINED ENV{MINGW_PATH})
    if(EXISTS "$ENV{MINGW_PATH}/g++.exe")
        set(CMAKE_CXX_COMPILER "$ENV{MINGW_PATH}/g++.exe")
        set(CMAKE_C_COMPILER "$ENV{MINGW_PATH}/gcc.exe")
        message(STATUS "Using MinGW from .env: $ENV{MINGW_PATH}")
    endif()
endif()

project(Bejeweled LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Helps VS Code IntelliSense

# Add cmake/ folder to module path for custom Find modules (like FindGLFW3.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ==============================================================================
# Dependencies
# ==============================================================================

# Qt6
if(DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
endif()
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Boost
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT "$ENV{BOOST_ROOT}")
    set(Boost_INCLUDE_DIR "$ENV{BOOST_ROOT}")
endif()
find_package(Boost REQUIRED)

# GLEW (Optional)
if(DEFINED ENV{GLEW_DIR} AND NOT "$ENV{GLEW_DIR}" STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{GLEW_DIR}")
    find_package(GLEW REQUIRED)
endif()

# GLFW (Optional)
if(DEFINED ENV{GLFW_DIR} AND NOT "$ENV{GLFW_DIR}" STREQUAL "")
    # Use our custom FindGLFW3.cmake which uses ENV{GLFW_DIR} as hint
    find_package(GLFW3 REQUIRED)
endif()

# ==============================================================================
# Project Targets
# ==============================================================================

qt_standard_project_setup()

# Collect sources from src directory
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

qt_add_executable(Bejeweled
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
)

target_link_libraries(Bejeweled
    PRIVATE
    Qt::Core
    Qt::Widgets
    Boost::headers
)

if(TARGET GLEW::GLEW)
    target_link_libraries(Bejeweled PRIVATE GLEW::GLEW)
endif()

if(TARGET GLFW3::GLFW3)
    target_link_libraries(Bejeweled PRIVATE GLFW3::GLFW3)
elseif(TARGET glfw)
    target_link_libraries(Bejeweled PRIVATE glfw)
endif()

# ==============================================================================
# Installation & Deployment
# ==============================================================================

include(GNUInstallDirs)

install(TARGETS Bejeweled
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Bejeweled
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
