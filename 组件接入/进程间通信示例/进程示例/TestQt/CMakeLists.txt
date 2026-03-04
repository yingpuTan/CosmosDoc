cmake_minimum_required(VERSION 3.10)
project(TestQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 使用 Qt5
# 如果系统找不到 Qt5，可以通过 CMAKE_PREFIX_PATH 指定 Qt5 安装路径
# 例如：cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.12.2/msvc2017_64" ..
# 或者设置环境变量 CMAKE_PREFIX_PATH

# 可以在这里设置默认的 Qt5 路径（如果知道的话）
# set(CMAKE_PREFIX_PATH "C:/Qt/5.12.2/msvc2017_64" ${CMAKE_PREFIX_PATH})

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# 显示找到的 Qt5 路径（用于调试）
if(Qt5_FOUND)
    message(STATUS "找到 Qt5 版本: ${Qt5_VERSION}")
    message(STATUS "Qt5 安装路径: ${Qt5_DIR}")
    get_target_property(QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
    if(QT_QMAKE_EXECUTABLE)
        get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
        get_filename_component(QT_ROOT_DIR "${QT_BIN_DIR}" DIRECTORY)
        message(STATUS "Qt5 根目录: ${QT_ROOT_DIR}")
    endif()
else()
    message(FATAL_ERROR "未找到 Qt5！请使用以下方式之一指定 Qt5 路径：\n"
        "1. 命令行参数: cmake -DCMAKE_PREFIX_PATH=\"C:/Qt/5.12.2/msvc2017_64\" ..\n"
        "2. 设置环境变量 CMAKE_PREFIX_PATH\n"
        "3. 在 CMakeLists.txt 中取消注释 CMAKE_PREFIX_PATH 设置")
endif()

if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# PipeClient 静态/动态库源码（直接一起编进 TestQt，便于 Demo 使用）
add_subdirectory(../PipeClient PipeClient_build)

add_executable(TestQt
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(TestQt PRIVATE Qt5::Widgets PipeClient)

if(UNIX AND NOT APPLE)
    target_link_libraries(TestQt PRIVATE X11::X11)
endif()


