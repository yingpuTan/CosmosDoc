cmake_minimum_required(VERSION 3.10)

project(CosmosHostDemo)

# 设置SDK路径
set(COSMOS_SDK_PATH "./")

# 包含头文件
include_directories(${COSMOS_SDK_PATH}/include)
# 包含rapidjson头文件
include_directories(${COSMOS_SDK_PATH}/include/rapidjson)

set(SRC_FILE 
    "CCosmosHostApi.h"
    "main.cpp"
    "CCosmosHostApi.cpp"
    "platform.h"
    "platform.cpp"
)

add_executable(CosmosHostDemo
    ${SRC_FILE}
)

set_target_properties(CosmosHostDemo PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

if (WIN32)
    target_compile_definitions(CosmosHostDemo PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(CosmosHostDemo PRIVATE Threads::Threads)
    if (UNIX)
        # dlopen/dlsym
        target_link_libraries(CosmosHostDemo PRIVATE dl)
    endif()
endif()

