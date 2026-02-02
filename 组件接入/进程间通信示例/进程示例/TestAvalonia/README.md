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

1. 运行程序后，会弹出连接对话框
2. 输入 RPC 管道名称（例如：`cosmos_pipe`）
3. 点击"确定"建立连接

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
4. **连接对话框**：启动时自动弹出连接对话框
5. **移除 Windows 特定代码**：移除了 Win32 API 调用，实现真正的跨平台

## 注意事项

1. 确保 `libPipeClient.so`（Linux/macOS）或 `PipeClient.dll`（Windows）在运行时可访问
2. 管道名称需要与 Cosmos 主程序配置一致
3. 日志文件将保存在当前目录：`test_rpc_avalonia.log`
4. 在 Linux 上运行时，可能需要安装额外的依赖：
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libxcursor-dev
   
   # Fedora/RHEL
   sudo dnf install libX11-devel libXrandr-devel libXi-devel mesa-libGL-devel libXcursor-devel
   ```

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

