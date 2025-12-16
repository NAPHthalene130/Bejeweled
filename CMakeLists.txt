cmake_minimum_required(VERSION 3.16)
project(Bejeweled LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia 3DCore 3DRender 3DExtras 3DInput 3DLogic 3DAnimation 3DQuick 3DQuickAnimation 3DQuickExtras 3DQuickInput 3DQuickLogic 3DQuickRender 3DQuickScene2D 3DQuickScene3D)

# Boost（如有需要可取消注释并配置路径）
# set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/boost")
# set(Boost_NO_SYSTEM_PATHS ON)
# find_package(Boost REQUIRED)

# GLEW、OpenSSL、GLFW等可按需添加

# 源文件收集
file(GLOB_RECURSE SOURCES
    src/*.cpp
)
file(GLOB_RECURSE HEADERS
    include/*.h
    src/*.h
)

option(BUILD_SHARED_LIB "Build core as shared library (DLL)" ON)

set(MAIN_SRC "${CMAKE_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM SOURCES ${MAIN_SRC})

if(BUILD_SHARED_LIB)
    add_library(BejeweledLib SHARED ${SOURCES} ${HEADERS})
    add_executable(Bejeweled ${MAIN_SRC})
    target_link_libraries(Bejeweled PRIVATE BejeweledLib)
    # expose include dirs for consumers
    target_include_directories(BejeweledLib PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )
else()
    add_executable(Bejeweled ${SOURCES} ${HEADERS})
endif()

if(BUILD_SHARED_LIB)
    target_include_directories(Bejeweled PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )
else()
    target_include_directories(Bejeweled PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
    )
endif()

# 传递源码根目录给代码
add_definitions(-DCODE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")


# 链接Qt3D及其所有常用子模块，确保动画和3D功能完整
if(BUILD_SHARED_LIB)
    target_link_libraries(BejeweledLib PUBLIC Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::3DCore Qt6::3DRender Qt6::3DExtras Qt6::3DInput Qt6::3DLogic Qt6::3DAnimation Qt6::3DQuick Qt6::3DQuickAnimation Qt6::3DQuickExtras Qt6::3DQuickInput Qt6::3DQuickLogic Qt6::3DQuickRender Qt6::3DQuickScene2D Qt6::3DQuickScene3D)
else()
    target_link_libraries(Bejeweled PRIVATE Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::3DCore Qt6::3DRender Qt6::3DExtras Qt6::3DInput Qt6::3DLogic Qt6::3DAnimation Qt6::3DQuick Qt6::3DQuickAnimation Qt6::3DQuickExtras Qt6::3DQuickInput Qt6::3DQuickLogic Qt6::3DQuickRender Qt6::3DQuickScene2D Qt6::3DQuickScene3D)
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
        Qt63DAnimation.dll
        Qt63DInput.dll
        Qt63DLogic.dll
        Qt63DQuick.dll
        Qt63DQuickAnimation.dll
        Qt63DQuickExtras.dll
        Qt63DQuickInput.dll
        Qt63DQuickLogic.dll
        Qt63DQuickRender.dll
        Qt63DQuickScene2D.dll
        Qt63DQuickScene3D.dll
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
        Qt6Quick3DXr.dll
        Qt6QuickWidgets.dll
        Qt6Qml.dll
        Qt6QmlCore.dll
        Qt6QmlModels.dll
        Qt6QmlWorkerScript.dll
        Qt6QmlNetwork.dll
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
    set(QT_BIN_DIR "C:/Qt/6.10.1/mingw_64/bin")
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
add_custom_command(TARGET Bejeweled POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Bejeweled>/assets)

# 如果存在 resources 目录则一并复制到输出目录，供运行时读取
if(EXISTS "${CMAKE_SOURCE_DIR}/resources")
    add_custom_command(TARGET Bejeweled POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:Bejeweled>/resources)
endif()

# 确保代码可以使用项目源目录宏（ResourceUtils 使用 PROJECT_SOURCE_DIR）
add_definitions(-DPROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
