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
            if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
            {
                // 获取命令行参数
                string[] args = desktop.Args?.ToArray() ?? System.Environment.GetCommandLineArgs().Skip(1).ToArray();
                
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
    }
}

