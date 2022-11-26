cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(VTK_Design)

find_package(GDAL CONFIG REQUIRED)

find_package(VTK COMPONENTS
    CommonColor
    CommonCore
    IOGeometry
    FiltersSources
    InteractionStyle
    RenderingContextOpenGL2
    RenderingCore
    RenderingFreeType
    RenderingGL2PSOpenGL2
    RenderingOpenGL2
    GUISupportQt
    REQUIRED
)

find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    OpenGL
    OpenGLWidgets
    REQUIRED
)

if(NOT GDAL_FOUND)
    message(FATAL_ERROR "GDAL not found")
endif()

if(NOT VTK_FOUND)
    message(FATAL_ERROR "Unable to find the VTK build folder.")
endif()

if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Unable to find the Qt6.")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR true)

set(CMAKE_AUTOMOC ON) # Meta-Object Compiler
set(CMAKE_AUTORCC ON) # Resource Compiler
set(CMAKE_AUTOUIC ON) # User Interface Compiler

# aux_source_directory(. SRC_LIST)

# Prevent a "command line is too long" failure in Windows.
set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")

add_executable(model_viewer WIN32 main.cc ViewWidget3D.h ViewWidget3D.cpp ViewWidget3D.ui ModelViewer.h ModelViewer.cpp ModelViewer.ui)

target_link_libraries(model_viewer PRIVATE
    GDAL::GDAL
    ${VTK_LIBRARIES}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::OpenGL
)

# vtk_module_autoinit is needed
vtk_module_autoinit(
    TARGETS model_viewer
    MODULES ${VTK_LIBRARIES}
)
