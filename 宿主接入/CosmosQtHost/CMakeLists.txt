cmake_minimum_required(VERSION 3.10)

project(CosmosQtHost LANGUAGES CXX)

# Cosmos SDK 位置：复用 CosmosHost 下的 SDK 目录
set(COSMOS_SDK_PATH "../CosmosHost")

# 本工程本地 include 目录（结构与 CosmosHost 一致）
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 还保留对 CosmosHost 的头文件引用（如 platform.h）
include_directories(${COSMOS_SDK_PATH})
include_directories(${COSMOS_SDK_PATH}/include)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# 设置 Qt5 查找路径
# 优先级：1. CMAKE_PREFIX_PATH (cmake 参数) 2. QT_DIR 环境变量 3. 自动查找常见路径

# 如果通过环境变量设置了 Qt 路径
if(DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
    message(STATUS "使用环境变量 QT_DIR: $ENV{QT_DIR}")
endif()

# 如果 CMAKE_PREFIX_PATH 为空，尝试自动查找常见路径
if("${CMAKE_PREFIX_PATH}" STREQUAL "")
    # 尝试常见的 Qt5 安装路径
    if(WIN32)
        # Windows 常见路径
        set(QT5_POSSIBLE_PATHS
            "C:/Qt/5.15.2/msvc2019_64"
            "C:/Qt/5.15.2/msvc2017_64"
            "C:/Qt/5.12.12/msvc2017_64"
            "C:/Qt/5.9.9/msvc2017_64"
            "C:/Qt/Qt5.15.2/5.15.2/msvc2019_64"
            "C:/Qt/Qt5.12.12/5.12.12/msvc2017_64"
        )
    else()
        # Linux 常见路径
        set(QT5_POSSIBLE_PATHS
            "/usr/lib/x86_64-linux-gnu/qt5"
            "/usr/lib/qt5"
            "/opt/qt5"
        )
    endif()
    
    # 查找 Qt5Config.cmake 文件
    find_path(QT5_CMAKE_DIR 
        NAMES Qt5Config.cmake
        PATHS ${QT5_POSSIBLE_PATHS}
        PATH_SUFFIXES lib/cmake/Qt5
        NO_DEFAULT_PATH
    )
    
    if(QT5_CMAKE_DIR)
        get_filename_component(QT5_ROOT_DIR "${QT5_CMAKE_DIR}/../.." ABSOLUTE)
        list(APPEND CMAKE_PREFIX_PATH "${QT5_ROOT_DIR}")
        message(STATUS "自动找到 Qt5: ${QT5_ROOT_DIR}")
    endif()
endif()

# 查找 Qt5 各模块
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)

if(NOT Qt5_FOUND)
    message(FATAL_ERROR 
        "无法找到 Qt5！\n"
        "请执行以下操作之一：\n"
        "1. 设置环境变量 QT_DIR 指向 Qt5 安装目录（例如：C:/Qt/5.15.2/msvc2019_64）\n"
        "2. 使用 cmake -DCMAKE_PREFIX_PATH=\"C:/Qt/5.15.2/msvc2019_64\" ..\n"
        "3. 在 CMakeLists.txt 中手动设置 CMAKE_PREFIX_PATH"
    )
endif()

set(SRC_FILES
    main.cpp
    MainWindow.h
    MainWindow.cpp
    ${COSMOS_SDK_PATH}/platform.h
    ${COSMOS_SDK_PATH}/platform.cpp
)

add_executable(CosmosQtHost
    ${SRC_FILES}
)

# 链接 Qt5 库（显式链接 Widgets/Core/Gui，避免某些版本下的未解析符号）
target_link_libraries(CosmosQtHost PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
)

if (WIN32)
    target_compile_definitions(CosmosQtHost PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(CosmosQtHost PRIVATE Threads::Threads)
    if (UNIX)
        target_link_libraries(CosmosQtHost PRIVATE dl)
    endif()
endif()


