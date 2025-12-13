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

# Set VCPKG Toolchain if VCPKG_DIR is provided in .env
if(DEFINED ENV{VCPKG_DIR} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(EXISTS "$ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
        message(STATUS "Using vcpkg toolchain from .env: ${CMAKE_TOOLCHAIN_FILE}")
    else()
        message(WARNING "VCPKG_DIR set in .env but scripts/buildsystems/vcpkg.cmake not found at $ENV{VCPKG_DIR}")
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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Boost
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT "$ENV{BOOST_ROOT}")
    set(Boost_INCLUDE_DIR "$ENV{BOOST_ROOT}")
endif()
find_package(Boost REQUIRED)

# GLEW (via vcpkg)
find_package(GLEW REQUIRED)

# OpenSSL (via MSYS2)
find_package(OpenSSL REQUIRED)
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

target_include_directories(Bejeweled PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_include_directories(Bejeweled PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(Bejeweled PRIVATE ${OPENSSL_LIBRARIES})
target_link_libraries(Bejeweled
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
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
