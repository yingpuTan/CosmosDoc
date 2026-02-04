cmake_minimum_required(VERSION 3.10)
project(TestQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 使用 Qt5
find_package(Qt5 COMPONENTS Widgets REQUIRED)

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


