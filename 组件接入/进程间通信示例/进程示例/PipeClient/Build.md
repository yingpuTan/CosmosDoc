# 构建说明

## 概述

PipeClient 项目支持 Windows 和 Linux 两个平台，使用命名管道技术进行跨进程通信，并与 C# `NamedPipeServerStream`（包括 .NET 7）完全兼容。

### 主要特性

1. **跨进程通信技术**：
   - Windows: 使用命名管道 (Named Pipe)
   - Linux: 使用 Unix Domain Socket（与 C# NamedPipeServerStream 兼容）

2. **文件系统操作**：所有文件系统相关操作已实现跨平台支持

3. **时间处理**：使用跨平台的时间 API

4. **UUID 生成**：Windows 使用 CoCreateGuid，Linux 使用 uuid_generate

## Windows 构建

### 构建要求

- Visual Studio 2019 或更高版本（支持 C++11）
- Windows SDK 10.0 或更高版本

### 构建步骤

1. 使用 Visual Studio 打开 `PipeClient.vcxproj`
2. 选择配置（Debug/Release）和平台（Win32/x64）
3. 生成解决方案

构建完成后，会在输出目录下生成 `PipeClient.dll` 或 `PipeClient.exe`。

## Linux 构建

### 构建要求

- CMake 3.10 或更高版本
- GCC 或 Clang 编译器（支持 C++11）
- pthread 库（通常系统自带）
- uuid 开发库

### 安装依赖

在 Ubuntu/Debian 上安装依赖：

```bash
sudo apt-get install build-essential cmake uuid-dev
```

在 CentOS/RHEL 上安装依赖：

```bash
sudo yum install gcc-c++ cmake libuuid-devel
```

### 构建步骤

```bash
cd PipeClient
mkdir build
cd build
cmake ..
make
```

构建完成后，会在 `build` 目录下生成 `libPipeClient.so` 共享库。

## 使用说明

### 管道名称差异

- **Windows**: 管道名称格式为 `\\.\pipe\<name>`
- **Linux**: Unix Domain Socket 路径格式为 `/tmp/<name>`，也可以只提供名称（如 `"MyPipe"`），代码会自动转换为 `/tmp/MyPipe`

在调用 `InitClient` 时，需要根据平台传入不同的管道名称：

```cpp
#ifdef _WIN32
    const char* pipeName = "\\\\.\\pipe\\MyPipe";
#else
    // Linux: 可以使用名称或完整路径
    const char* pipeName = "MyPipe";  // 或 "/tmp/MyPipe"
#endif
InitClient(pipeName, strlen(pipeName), logPath, strlen(logPath));
```

**注意**：Linux 端如果只提供名称（如 `"MyPipe"`），会自动转换为 `/tmp/MyPipe`。如果与 C# `.NET 7` 的 `NamedPipeServerStream` 通信，可能需要匹配 C# 实际创建的 socket 文件路径。

### 服务器端要求

#### Windows 服务器端

服务器端使用 Windows API 创建命名管道：

```cpp
HANDLE hPipe = CreateNamedPipe(
    L"\\\\.\\pipe\\MyPipe",
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES,
    4096, 4096, 0, NULL);
```

#### Linux 服务器端

**使用 Unix Domain Socket（与 C# NamedPipeServerStream 兼容）**

服务器端需要使用 `socket()` 和 `bind()` 创建 Unix Domain Socket：

```c
int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
struct sockaddr_un addr;
memset(&addr, 0, sizeof(addr));
addr.sun_family = AF_UNIX;
strncpy(addr.sun_path, "/tmp/MyPipe", sizeof(addr.sun_path) - 1);
bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
listen(server_fd, 5);
// Accept connections and communicate
```

**注意**：如果使用 C# `NamedPipeServerStream`，它会自动创建和管理 socket 文件，C++ 客户端只需要连接即可。

## 注意事项

### Windows

1. 命名管道由系统管理，不需要手动创建文件
2. 管道名称使用 `\\.\pipe\` 前缀
3. 支持多个客户端同时连接（通过 PIPE_UNLIMITED_INSTANCES）

### Linux

1. **使用 Unix Domain Socket**（与 C# NamedPipeServerStream 兼容）
   - 客户端使用 `socket()` 和 `connect()` 连接
   - 服务器端使用 `socket()`, `bind()`, `listen()`, `accept()` 接受连接
   - Socket 文件路径通常在 `/tmp` 目录下

2. **管道名称格式**：
   - 如果只提供名称（如 `"MyPipe"`），会自动转换为 `/tmp/MyPipe`
   - 也可以提供完整路径（如 `"/tmp/MyPipe"`）
   - C# `NamedPipeServerStream`（.NET 7）可能使用特定命名格式（如 `.NET-Core-Pipe-{name}-{pid}-{random}`），如果连接失败需要查找实际路径

3. **Socket 文件管理**：
   - Socket 文件由服务器端创建
   - 程序退出后可能需要手动清理（`unlink()` 或 `rm` 命令）
   - 确保有 `/tmp` 目录的读写权限

4. 所有日志文件路径使用 `/` 作为路径分隔符（Linux 标准）

5. Unix Domain Socket 支持全双工通信，类似于 Windows 命名管道

## 运行时依赖

### Linux 运行时依赖

编译生成的 `.so` 文件在纯净 Linux 系统下需要以下运行时库：

1. **libpthread.so** - POSIX 线程库
   - 通常包含在 `glibc` 中，大多数 Linux 发行版默认安装
   - 如果缺失，安装：`sudo apt-get install libc6` (Ubuntu/Debian) 或 `sudo yum install glibc` (CentOS/RHEL)

2. **libuuid.so** - UUID 生成库
   - 可能需要额外安装运行时库
   - Ubuntu/Debian: `sudo apt-get install libuuid1`
   - CentOS/RHEL: `sudo yum install libuuid`

3. **标准 C++ 库** (libstdc++.so)
   - 通常随编译器安装
   - 如果缺失：`sudo apt-get install libstdc++6` (Ubuntu/Debian)

### 检查依赖关系

使用 `ldd` 命令检查 `.so` 文件的依赖：

```bash
ldd libPipeClient.so
```

示例输出：
```
linux-vdso.so.1 (0x00007fff12345000)
libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f1234567890)
libuuid.so.1 => /lib/x86_64-linux-gnu/libuuid.so.1 (0x00007f1234567890)
libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f1234567890)
libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f1234567890)
libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f1234567890)
```

如果某个库显示 "not found"，则需要安装对应的运行时库。

### 最小化部署

如果需要在最小化 Linux 系统上运行，建议：

1. **检查目标系统**：先使用 `ldd` 检查依赖
2. **安装缺失库**：根据 `ldd` 输出安装缺失的库
3. **静态链接选项**（可选）：如果需要完全独立，可以考虑静态链接某些库，但这会增加文件大小

### 静态链接（可选）

如果需要减少运行时依赖，可以在 CMakeLists.txt 中使用静态链接：

```cmake
# 静态链接 pthread（不推荐，会增加文件大小）
target_link_options(PipeClient PRIVATE -static-libgcc -static-libstdc++)
```

**注意**：完全静态链接可能导致兼容性问题，通常不推荐。

## 测试

建议在对应平台环境下进行完整测试，确保：

- 连接功能正常
- 消息发送和接收正常
- 日志记录功能正常
- 文件清理功能正常
- 在目标系统上检查运行时依赖是否满足
- 与 C# `.NET 7` `NamedPipeServerStream` 的通信测试（如果适用）

## 跨平台兼容性

代码使用条件编译（`#ifdef _WIN32`）来区分 Windows 和 Linux 平台，确保：

- 相同的 API 接口
- 相同的数据协议格式
- 相同的业务逻辑
- 与 C# `NamedPipeServerStream` 的完全兼容（Windows 和 Linux）

只需要在调用时根据平台传入不同的管道名称即可。Linux 端支持自动路径转换，使用更简单。

## ✅ C# NamedPipeServerStream 兼容性

**当前实现已支持与 C# `NamedPipeServerStream` 通信！**

### 实现说明

- **Linux 端**：使用 **Unix Domain Socket**（与 C# 兼容）
- **Windows 端**：使用 **Named Pipe**（与 C# 兼容）
- 两端都可以与对应平台的 C# `NamedPipeServerStream` 正常通信

### 使用示例

**C# 服务器端（Linux，.NET 7）**：
```csharp
using System.IO.Pipes;

using (var pipeServer = new NamedPipeServerStream(
    "MyPipe", 
    PipeDirection.InOut,
    1,  // maxNumberOfServerInstances
    PipeTransmissionMode.Byte,
    PipeOptions.None))
{
    pipeServer.WaitForConnection();
    // 通信代码
}
```

**C++ 客户端（Linux）**：
```cpp
// 管道名称可以是 "MyPipe" 或 "/tmp/MyPipe"
// 代码会自动将 "MyPipe" 转换为 "/tmp/MyPipe"
InitClient("MyPipe", strlen("MyPipe"), logPath, strlen(logPath));
```

**注意**：C# `NamedPipeServerStream`（.NET 7）在 Linux 上创建的 socket 文件路径可能与直接提供的名称不同。如果连接失败，可以：
1. 检查 `/tmp` 目录下的 socket 文件（通常格式为 `.NET-Core-Pipe-{name}-{pid}-{random}`）
2. 使用完整路径：`"/tmp/MyPipe"` 或 C# 实际创建的路径
3. 使用 `find /tmp -name "*pipe*"` 查找实际创建的 socket 文件
4. 查看日志文件中的错误信息，了解尝试连接的路径

详细说明请参考 `CSharp_Compatibility.md` 文件。

