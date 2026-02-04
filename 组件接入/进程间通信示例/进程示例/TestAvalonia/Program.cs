using Avalonia;
using System;
using System.IO;

namespace TestAvalonia
{
    internal class Program
    {
        // Initialization code. Don't use any Avalonia, third-party APIs or any
        // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
        // yet and stuff might break.
#if WINDOWS
        [STAThread]
#endif
        public static void Main(string[] args)
        {
            // 添加全局异常处理
            AppDomain.CurrentDomain.UnhandledException += (sender, e) =>
            {
                try
                {
                    string logPath = "test_avalonia_crash.log";
                    File.AppendAllText(logPath, $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] 未处理的异常: {e.ExceptionObject}\n");
                    if (e.ExceptionObject is Exception ex)
                    {
                        File.AppendAllText(logPath, $"异常类型: {ex.GetType().Name}\n");
                        File.AppendAllText(logPath, $"异常消息: {ex.Message}\n");
                        File.AppendAllText(logPath, $"堆栈跟踪: {ex.StackTrace}\n");
                    }
                }
                catch { }
            };

            try
            {
                BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
            }
            catch (Exception ex)
            {
                try
                {
                    string logPath = "test_avalonia_crash.log";
                    File.AppendAllText(logPath, $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] 启动异常: {ex.GetType().Name}\n");
                    File.AppendAllText(logPath, $"异常消息: {ex.Message}\n");
                    File.AppendAllText(logPath, $"堆栈跟踪: {ex.StackTrace}\n");
                }
                catch { }
                throw;
            }
        }

        // Avalonia configuration, don't remove; also used by visual designer.
        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .WithInterFont()
                .LogToTrace();
    }
}

