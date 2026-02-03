# Cosmos RPC 跨平台 GUI 测试 Demo (Avalonia)

这是一个基于 **Avalonia UI** 框架的跨平台图形界面应用程序，用于测试 Cosmos RPC 通信功能。支持 Windows、Linux 和 macOS 平台。

## 功能特性

- ✅ **跨平台支持**：Windows、Linux、macOS
- ✅ **现代化 UI**：基于 Avalonia 11.0 的 Fluent 主题
- ✅ **完整的 RPC 功能**：
  - RPC 管道连接
  - Invoke（同步/异步调用）
  - Notify（通知）
  - Push（推送）
  - Subscribe（订阅）
- ✅ **账户查询功能**
- ✅ **主题切换支持**
- ✅ **组件通信测试**
- ✅ **实时状态显示**

## 项目结构

```
TestAvalonia/
├── App.axaml              # 应用程序定义
├── App.axaml.cs          # 应用程序代码
├── MainWindow.axaml      # 主窗口 XAML
├── MainWindow.axaml.cs   # 主窗口代码
├── Program.cs            # 程序入口
├── TestAvalonia.csproj   # 项目配置
├── app.manifest          # Windows 清单文件
└── README.md            # 说明文档
```

## 编译和运行

### 前置要求

- .NET 7.0 SDK 或更高版本
- 确保已安装 Avalonia 设计器（可选，用于设计时预览）

### Windows

```bash
cd TestAvalonia
dotnet restore
dotnet build
dotnet run
```

### Linux

```bash
cd TestAvalonia
dotnet restore
dotnet build
dotnet run
```

### macOS

```bash
cd TestAvalonia
dotnet restore
dotnet build
dotnet run
```

## 使用方法

### 启动应用

#### 方式一：使用命令行参数（推荐）

```bash
# Windows
dotnet run "pipe_name" "parentHandle|width|height"

# Linux/macOS
dotnet run "pipe_name" "parentHandle|width|height"
```

**参数说明：**
- 第一个参数：RPC 管道名称（例如：`cosmos_pipe`）
- 第二个参数：窗口信息，格式为 `parentHandle|width|height`
  - `parentHandle`: 父窗口句柄（整数）
  - `width`: 窗口宽度（像素）
  - `height`: 窗口高度（像素）

**示例：**
```bash
# 仅连接管道，不嵌入窗口
dotnet run "cosmos_pipe"

# 连接管道并嵌入到父窗口
dotnet run "cosmos_pipe" "123456|800|600"
```

#### 方式二：交互式启动

如果不提供命令行参数，程序启动后会弹出连接对话框：
1. 输入 RPC 管道名称（例如：`cosmos_pipe`）
2. 点击"确定"建立连接

**注意：** 交互式启动不支持窗口嵌入功能，窗口嵌入必须通过命令行参数提供窗口信息。

### 功能说明

#### 左侧面板：测试账户和 RPC 功能

1. **第一步：通知对方来查询和订阅**
   - 发送 Notify 消息，通知对方进行查询和订阅操作

2. **第二步：推送账户信息**
   - 推送账户信息更新（需要先订阅）

3. **同步 Invoke 示例**
   - 发送同步 RPC 请求，等待响应（会阻塞界面）

4. **异步 Invoke 示例**
   - 发送异步 RPC 请求，不阻塞界面

#### 右侧面板：主题和组件通信测试

1. **第一步：获取皮肤资源**
   - 从服务器获取主题资源字典

2. **第二步：获取当前皮肤主题**
   - 获取当前使用的主题并应用

3. **组件通信透传测试**
   - 发送内容：要发送的文本内容
   - 组名：目标组件组名
   - 类型：选择"请求"或"推送"
   - 全局发送：向全局范围发送消息
   - 组发送：向指定组发送消息

4. **状态显示区域**
   - 实时显示所有操作和回调的状态信息

## 回调处理

程序会自动处理以下回调：

- **On_Invoke**: 处理 RPC 调用请求（如账户查询）
- **On_Notify**: 处理通知消息（如窗口尺寸调整、关闭、主题切换等）
- **On_Push**: 处理推送消息（如账户信息更新）
- **On_Subscribe**: 处理订阅请求

所有回调信息都会显示在右侧的状态显示区域。

## 依赖项

- **.NET 7.0** 或更高版本
- **Avalonia 11.0.0** - 跨平台 UI 框架
- **Avalonia.Desktop 11.0.0** - 桌面应用支持
- **Avalonia.Themes.Fluent 11.0.0** - Fluent 主题
- **Avalonia.Fonts.Inter 11.0.0** - Inter 字体
- **RpcWrapper** - 本地项目引用
- **Newtonsoft.Json 13.0.3** - JSON 处理

## 与 TestWpf 的对比

| 功能 | TestWpf | TestAvalonia |
|------|---------|--------------|
| 平台支持 | Windows only | Windows + Linux + macOS |
| UI 框架 | WPF | Avalonia |
| 窗口嵌入 | ✅ (Win32 API) | ❌ (跨平台不支持) |
| 主题切换 | ✅ | ✅ |
| 核心 RPC 功能 | ✅ | ✅ |
| 现代化 UI | ❌ | ✅ (Fluent Design) |
| 状态显示 | ❌ | ✅ |

## 主要改进

1. **跨平台支持**：完全支持 Windows、Linux 和 macOS
2. **现代化 UI**：使用 Avalonia 的 Fluent 主题，界面更美观
3. **状态显示**：添加了实时状态显示区域，方便调试
4. **命令行参数支持**：支持从命令行参数获取管道名和窗口信息（与 TestWpf 一致）
5. **窗口嵌入功能**：支持将窗口嵌入到父窗口中（Windows 平台使用 Win32 API）
6. **智能连接**：如果提供了命令行参数，自动连接；否则显示连接对话框

## 窗口嵌入功能

### Windows 平台

在 Windows 平台上，程序支持将窗口嵌入到父窗口中：

1. **通过命令行参数提供窗口信息**：
   ```bash
   dotnet run "pipe_name" "parentHandle|width|height"
   ```

2. **窗口嵌入行为**：
   - 窗口会自动设置为子窗口样式（无边框、不可调整大小）
   - 窗口会嵌入到指定的父窗口中
   - 窗口大小会根据参数自动调整
   - 窗口不会显示在任务栏中

3. **动态调整大小**：
   - 当收到 `setsize` 通知时，窗口会自动调整大小
   - 支持 DPI 缩放和窗口缩放

### Linux 平台

Linux 平台使用 **X11 API** 实现窗口嵌入功能：

1. **通过命令行参数提供窗口信息**：
   ```bash
   dotnet run "pipe_name" "parentX11WindowId|width|height"
   ```

2. **窗口嵌入行为**：
   - 使用 `XReparentWindow` 将窗口嵌入到父窗口
   - 使用 `XSetWindowBorderWidth` 设置无边框
   - 使用 `XMoveResizeWindow` 设置窗口大小和位置
   - 窗口不会显示在任务栏中

3. **动态调整大小**：
   - 当收到 `setsize` 通知时，窗口会自动调整大小
   - 支持 DPI 缩放和窗口缩放

4. **系统依赖**：
   - 需要安装 X11 开发库：
     ```bash
     # Ubuntu/Debian
     sudo apt-get install libx11-dev
     
     # Fedora/RHEL
     sudo dnf install libX11-devel
     ```

### macOS 平台

macOS 平台的窗口嵌入功能需要额外的实现（当前版本暂不支持）。如果需要，可以使用 Cocoa API 来实现。

## 注意事项

1. 确保 `libPipeClient.so`（Linux/macOS）或 `PipeClient.dll`（Windows）在运行时可访问
2. 管道名称需要与 Cosmos 主程序配置一致
3. 日志文件将保存在当前目录：`test_rpc_avalonia.log`
4. 窗口嵌入功能在 Windows 和 Linux 平台上支持，macOS 平台暂不支持
5. 在 Linux 上运行时，需要安装 X11 开发库（用于窗口嵌入功能）：
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libxcursor-dev
   
   # Fedora/RHEL
   sudo dnf install libX11-devel libXrandr-devel libXi-devel mesa-libGL-devel libXcursor-devel
   ```
   
   注意：如果只需要基本功能（不嵌入窗口），可以只安装 Avalonia 运行时的依赖。

## 示例输出

状态显示区域会显示类似以下的信息：

```
14:30:15 - ✓ 连接成功: cosmos_pipe
14:30:20 - [回调] 收到 Invoke 请求: qry_account
14:30:20 -   → 返回账户 123456 的信息
14:30:25 - ✓ 异步 Invoke 成功
14:30:25 -   响应 ID: abc123...
14:30:25 -   响应码: 0
```

## 故障排除

### 连接失败

- 检查管道名称是否正确
- 确保 Cosmos 主程序正在运行
- 检查日志文件 `test_rpc_avalonia.log`

### Linux 上无法启动

- 确保安装了必要的系统依赖（见上方）
- 检查 .NET 运行时是否正确安装
- 查看控制台错误信息

### 主题不生效

- 确保先执行"第一步：获取皮肤资源"
- 检查服务器是否返回了有效的主题资源

## 许可证

本项目遵循与 Cosmos 项目相同的许可证。

