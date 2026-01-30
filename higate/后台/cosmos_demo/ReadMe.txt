编译器版本和c++版本：gcc 5.4.0。 C++17

‌1.依赖安装‌
‌     librdkafka‌：Kafka C++客户端库（sudo apt-get install librdkafka-dev）‌
     ‌protobuf‌：Protocol Buffers序列化库（需自行安装对应版本）
2.安装完成protobuf，需要将subcenter.proto和rpc.proto生成对应的.cc和.h文件。

然后执行CMakeLists.txt


bin目录下，是编译完成产物，通过tool.ini文件配置连接站点等参数。