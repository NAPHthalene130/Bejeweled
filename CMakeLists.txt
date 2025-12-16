cmake_minimum_required(VERSION 3.19)

# ==============================================================================
# 环境配置（项目前）
# ==============================================================================
# 读取 .vscode/.env 文件以设置依赖项的环境变量
# 我们在 project() 之前执行此操作，以便根据需要设置编译器。
if(EXISTS "${CMAKE_SOURCE_DIR}/.vscode/.env")
    message(STATUS "Loading configuration from .vscode/.env file...")
    file(STRINGS "${CMAKE_SOURCE_DIR}/.vscode/.env" ConfigContents)
    foreach(line ${ConfigContents})
        # 忽略注释和空行
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

# 如果提供了 MINGW_PATH 且尚未设置，则设置编译器
if(NOT DEFINED CMAKE_CXX_COMPILER AND DEFINED ENV{MINGW_PATH})
    if(EXISTS "$ENV{MINGW_PATH}/g++.exe")
        set(CMAKE_CXX_COMPILER "$ENV{MINGW_PATH}/g++.exe")
        set(CMAKE_C_COMPILER "$ENV{MINGW_PATH}/gcc.exe")
        message(STATUS "Using MinGW from .env: $ENV{MINGW_PATH}")
    endif()
endif()

# 如果 .env 中提供了 VCPKG_DIR，则设置 VCPKG 工具链
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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 帮助 VS Code IntelliSense
 
 # 将 cmake/ 文件夹添加到模块路径，以便使用自定义查找模块（如 FindGLFW3.cmake）
 list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
 
 
 
 # ==============================================================================
 # 依赖项
 # ==============================================================================
 
 # Qt6
 if(DEFINED ENV{QT_DIR})
     list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
 endif()
 find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network 3DCore 3DRender 3DExtras Multimedia)
 
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
 # GLFW (可选)
 if(DEFINED ENV{GLFW_DIR} AND NOT "$ENV{GLFW_DIR}" STREQUAL "")
     # 使用我们的自定义 FindGLFW3.cmake，它使用 ENV{GLFW_DIR} 作为提示
     find_package(GLFW3 REQUIRED)
 endif()
 
 # ==============================================================================
 # 项目目标
 # ==============================================================================
 
 qt_standard_project_setup()
 
 # 从 src 目录收集源文件
 # 注意：如果添加新文件，请重新运行 CMake 或修改此文件。
 # 需要检测新添加的窗口部件和组件
 file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")


qt_add_executable(Bejeweled
    WIN32 MACOSX_BUNDLE
    ${SOURCES}
)

target_include_directories(Bejeweled PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Define PROJECT_SOURCE_DIR for resource loading
target_compile_definitions(Bejeweled PRIVATE PROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")

target_include_directories(Bejeweled PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(Bejeweled PRIVATE ${OPENSSL_LIBRARIES})
target_link_libraries(Bejeweled
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Network
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DExtras
    Qt6::Multimedia
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
