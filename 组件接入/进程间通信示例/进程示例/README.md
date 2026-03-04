# 进程示例说明

该目录包含 Cosmos 进程间通信相关的进程侧示例工程，包括：

- `PipeClient`：跨平台管道/Unix Domain Socket 客户端库
- `TestAvalonia`：基于 Avalonia 的跨平台 GUI 示例进程
- `TestQt`：基于 Qt 的示例进程
- `TestWpf` / `Test`：基于 WPF/MFC 的 Windows 示例进程

## Linux 平台注意事项

在 **Linux 系统** 下编译上述示例进程（如 `TestAvalonia`、`TestQt` 等）后：

1. 请确保对生成的可执行文件显式赋予可执行权限，例如：

   ```bash
   chmod +x ./TestAvalonia
   # 或者
   chmod +x ./TestQt
   ```

2. 如果未为可执行文件赋予执行权限，当 Cosmos 组件通过“进程间通信示例”启动该进程时，会出现 **“无执行权限/Permission denied”** 等错误，导致进程无法正常拉起。


