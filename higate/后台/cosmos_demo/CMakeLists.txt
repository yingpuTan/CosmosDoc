cmake_minimum_required(VERSION 3.10)
project(kafka_pb_demo)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 寻找Protobuf和librdkafka库
find_package(Protobuf REQUIRED)
find_library(LIBRDKAFKA_LIBRARY NAMES rdkafka)
find_path(LIBRDKAFKA_INCLUDE_DIR NAMES rdkafkacpp.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(main main.cpp subcenter.pb.cc rpc.pb.cc)
target_include_directories(main PRIVATE . ${PROTOBUF_INCLUDE_DIRS})
target_link_libraries(main ${PROTOBUF_LIBRARIES} rdkafka++ pthread z ssl crypto rt)