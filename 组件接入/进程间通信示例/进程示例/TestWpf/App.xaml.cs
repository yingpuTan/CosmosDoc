using System.Configuration;
using System.Data;
using System.Runtime.InteropServices;
using System.Windows;

namespace TestWpf
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        [DllImport("kernel32")]
        static extern bool AllocConsole();

        protected override void OnStartup(StartupEventArgs e)
        {
            //AllocConsole();
            //Console.ReadKey();
            base.OnStartup(e);

            // 解析命令行参数
            string[] args = e.Args;
            if (args.Length > 0)
            {
                MainWindow mainWindow = new MainWindow();
                if (args.Length > 1)
                {
                    mainWindow._pipeName = args[0];
                    mainWindow._wndInfo = args[1];
                }
                mainWindow.Show();
            }
            else
            {
                // 没有命令行参数
                MainWindow mainWindow = new MainWindow();
                mainWindow.Show();
            }
        }
    }
}
