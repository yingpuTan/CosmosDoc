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

find_package(Qt5 COMPONENTS Widgets REQUIRED)

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

target_link_libraries(CosmosQtHost PRIVATE Qt5::Widgets)

if (WIN32)
    target_compile_definitions(CosmosQtHost PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(CosmosQtHost PRIVATE Threads::Threads)
    if (UNIX)
        target_link_libraries(CosmosQtHost PRIVATE dl)
    endif()
endif()


