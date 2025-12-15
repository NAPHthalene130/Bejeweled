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

# Qt3D is optional: try to find components, but don't fail if missing
find_package(Qt6 COMPONENTS 3DCore 3DRender 3DExtras)
if (Qt6_3DCore_FOUND AND Qt6_3DRender_FOUND AND Qt6_3DExtras_FOUND)
    set(USE_QT3D TRUE)
    message(STATUS "Qt3D components found; enabling 3D features.")
else()
    set(USE_QT3D FALSE)
    message(WARNING "Qt3D components not found; building without 3D features.")
endif()

# Heuristic fallback: if Qt3D config directory exists on disk, enable 3D support
if(NOT USE_QT3D)
    if(DEFINED Qt63DCore_DIR OR EXISTS "${CMAKE_PREFIX_PATH}/Qt63DCore/Qt63DCoreConfig.cmake" OR EXISTS "C:/Qt/6.10.1/mingw_64/lib/cmake/Qt63DCore/Qt63DCoreConfig.cmake")
        message(STATUS "Qt63DCore config detected on disk; enabling Qt3D features (fallback).")
        set(USE_QT3D TRUE)
    endif()
endif()

# Boost
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT "$ENV{BOOST_ROOT}")
    set(Boost_INCLUDE_DIR "$ENV{BOOST_ROOT}")
endif()
find_package(Boost)
if(NOT Boost_FOUND)
    message(WARNING "Boost not found; proceeding without Boost headers. Define BOOST_ROOT or install Boost to enable optional features.")
endif()

# GLEW (via vcpkg) — optional
find_package(GLEW)
if(GLEW_FOUND)
    message(STATUS "GLEW found; enabling related features.")
    set(USE_GLEW TRUE)
else()
    message(WARNING "GLEW not found; building without GLEW features.")
    set(USE_GLEW FALSE)
endif()

# OpenSSL (via MSYS2) — optional
find_package(OpenSSL)
if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found; enabling crypto features.")
    set(USE_OPENSSL TRUE)
else()
    message(WARNING "OpenSSL not found; building without OpenSSL. RSA operations will use a fallback.")
    set(USE_OPENSSL FALSE)
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

target_include_directories(Bejeweled PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Define PROJECT_SOURCE_DIR for resource loading
target_compile_definitions(Bejeweled PRIVATE PROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")

if(USE_OPENSSL)
    target_include_directories(Bejeweled PRIVATE ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(Bejeweled PRIVATE ${OPENSSL_LIBRARIES})
    target_compile_definitions(Bejeweled PRIVATE HAVE_OPENSSL=1)
else()
    target_compile_definitions(Bejeweled PRIVATE HAVE_OPENSSL=0)
endif()
target_link_libraries(Bejeweled
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
)

if(Boost_FOUND)
    target_link_libraries(Bejeweled PRIVATE Boost::headers)
endif()

if(USE_QT3D)
    target_link_libraries(Bejeweled PRIVATE Qt6::3DCore Qt6::3DRender Qt6::3DExtras)
    target_compile_definitions(Bejeweled PRIVATE HAVE_QT3D=1)
endif()

if(USE_GLEW)
    if(TARGET GLEW::GLEW)
        target_link_libraries(Bejeweled PRIVATE GLEW::GLEW)
    endif()
    target_compile_definitions(Bejeweled PRIVATE HAVE_GLEW=1)
else()
    target_compile_definitions(Bejeweled PRIVATE HAVE_GLEW=0)
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

# Windows: optionally run windeployqt after build to copy Qt DLLs next to the executable
option(USE_WINDEPLOYQT "Run windeployqt after build to bundle Qt DLLs (Windows only)" ON)
if (WIN32 AND USE_WINDEPLOYQT)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt)
    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET Bejeweled POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --qmldir "${CMAKE_SOURCE_DIR}" $<TARGET_FILE:Bejeweled>
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            COMMENT "Running windeployqt to deploy Qt DLLs alongside the executable"
        )
        message(STATUS "windeployqt found; will run after build to bundle runtime DLLs.")
    else()
        message(WARNING "windeployqt not found on PATH; Qt DLLs will not be auto-deployed. Install Qt tools or set USE_WINDEPLOYQT=OFF to silence this.")
    endif()
endif()
