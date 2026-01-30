using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace Cosmos.MockDebugger.App
{
    internal class Program
    {
        [STAThread]
        public static void Main(String[] args)
        {
            [DllImport("kernel32")]
            static extern bool AllocConsole();
      
            //AllocConsole();
            //Console.ReadKey();
            Debugger debugger = new Debugger(@"D:\git\itrader\Cosmos_yinhe\Source\Cosmos.App.Hithink.ComDemo\bin\Debug\net7.0-windows\\Cosmos.App.Hithink.ComDemo.dll", "Cosmos.App.Hithink.ComDemo.WpfComDemoGui");
            debugger.StartDebugger();
        }
    }
}
