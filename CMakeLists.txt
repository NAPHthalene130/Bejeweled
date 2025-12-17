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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network 3DCore 3DRender 3DExtras 3DInput Multimedia)

# Boost
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT "$ENV{BOOST_ROOT}")
    set(Boost_INCLUDE_DIR "$ENV{BOOST_ROOT}")
endif()
find_package(Boost REQUIRED)

# GLEW (via vcpkg)
find_package(GLEW REQUIRED)

# OpenSSL (via vcpkg)
if(DEFINED ENV{OPENSSL_ROOT_DIR})
    set(OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}")
endif()
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
# AudioManager moved to src/utils
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

# Link necessary Qt modules
if(BUILD_SHARED_LIB)
    target_link_libraries(BejeweledLib PUBLIC Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::3DCore Qt6::3DRender Qt6::3DExtras Qt6::3DInput)
else()
    target_link_libraries(Bejeweled PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::3DCore Qt6::3DRender Qt6::3DExtras Qt6::3DInput)
endif()


# 自动复制Qt DLL到输出目录（仅Windows，需本地Qt bin目录）
if (WIN32)
    set(QT_DLLS
        Qt6Widgets.dll
        Qt6Network.dll
        Qt6Core.dll
        Qt6Gui.dll
        Qt6Multimedia.dll
        Qt63DCore.dll
        Qt63DRender.dll
        Qt63DExtras.dll
        Qt63DInput.dll
        Qt6OpenGL.dll
        Qt6OpenGLWidgets.dll
        Qt6Quick.dll
        Qt6Quick3D.dll
        Qt6Quick3DAssetImport.dll
        Qt6Quick3DAssetUtils.dll
        Qt6Quick3DEffects.dll
        Qt6Quick3DGlslParser.dll
        Qt6Quick3DHelpers.dll
        Qt6Quick3DHelpersImpl.dll
        Qt6Quick3DIblBaker.dll
        Qt6Quick3DParticleEffects.dll
        Qt6Quick3DParticles.dll
        Qt6Quick3DRuntimeRender.dll
        Qt6Quick3DSpatialAudio.dll
        Qt6Quick3DUtils.dll
        
        Qt6QuickWidgets.dll
        Qt6Qml.dll
        Qt6QmlCore.dll
        Qt6QmlModels.dll
        Qt6QmlWorkerScript.dll
        
        Qt6QmlXmlListModel.dll
        Qt6QuickControls2.dll
        Qt6QuickLayouts.dll
        Qt6QuickShapes.dll
        Qt6QuickTemplates2.dll
        Qt6Gui.dll
        Qt6PrintSupport.dll
        Qt6Sql.dll
        Qt6Svg.dll
        Qt6SvgWidgets.dll
        Qt6Test.dll
        Qt6UiTools.dll
        Qt6Xml.dll
        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
        d3dcompiler_47.dll
        opengl32sw.dll
    )
    # 请根据你的实际Qt安装路径修改下方路径
    if(DEFINED ENV{QT_DIR})
        set(QT_BIN_DIR "$ENV{QT_DIR}/bin")
    else()
        set(QT_BIN_DIR "C:/Qt/6.10.1/mingw_64/bin")
    endif()
    foreach(DLL ${QT_DLLS})
        # Always copy Qt DLLs to the executable output directory after build.
        # Bind to the executable target to avoid creating a strong dependency
        # from the shared library back to the executable (which creates a cycle).
        add_custom_command(TARGET Bejeweled POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${QT_BIN_DIR}/${DLL}
                $<TARGET_FILE_DIR:Bejeweled>/${DLL})
    endforeach()
endif()

# 资源文件夹复制（如有）
# Assets folder moved to resources, removed legacy copy command

# 如果存在 resources 目录则一并复制到输出目录，供运行时读取
if(EXISTS "${CMAKE_SOURCE_DIR}/resources")
    add_custom_command(TARGET Bejeweled POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:Bejeweled>/resources)
endif()

# 确保代码可以使用项目源目录宏（ResourceUtils 使用 PROJECT_SOURCE_DIR）
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
