using System;
using System.IO;
using System.Linq;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;

namespace TestAvalonia
{
    public partial class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }

        public override void OnFrameworkInitializationCompleted()
        {
            try
            {
                if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
                {
                    // 获取命令行参数
                    string[] args = desktop.Args?.ToArray() ?? System.Environment.GetCommandLineArgs().Skip(1).ToArray();
                    
                    // 记录启动信息到备用日志（此时 MainWindow 还未创建）
                    try
                    {
                        string logPath = "test_avalonia_status_backup.log";
                        System.IO.File.AppendAllText(logPath, $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] App.OnFrameworkInitializationCompleted 开始\n");
                        System.IO.File.AppendAllText(logPath, $"命令行参数数量: {args.Length}\n");
                        for (int i = 0; i < args.Length; i++)
                        {
                            System.IO.File.AppendAllText(logPath, $"参数[{i}]: {args[i]}\n");
                        }
                    }
                    catch { }
                    
                    MainWindow mainWindow = new MainWindow();
                    
                    if (args.Length > 0)
                    {
                        mainWindow._pipeName = args[0];
                    }
                    
                    if (args.Length > 1)
                    {
                        mainWindow._wndInfo = args[1];
                    }
                    
                    desktop.MainWindow = mainWindow;
                }

                base.OnFrameworkInitializationCompleted();
            }
            catch (Exception ex)
            {
                try
                {
                    string logPath = "test_avalonia_crash.log";
                    System.IO.File.AppendAllText(logPath, $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] OnFrameworkInitializationCompleted 异常: {ex.GetType().Name}\n");
                    System.IO.File.AppendAllText(logPath, $"异常消息: {ex.Message}\n");
                    System.IO.File.AppendAllText(logPath, $"堆栈跟踪: {ex.StackTrace}\n");
                }
                catch { }
                throw;
            }
        }
    }
}

