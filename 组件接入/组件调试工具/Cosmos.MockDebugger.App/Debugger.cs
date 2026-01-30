using Cosmos.Engine.Wpf;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.MockDebugger.App
{
    public class Debugger
    {
        public Debugger(string dllPath, string entry) 
        {
            if(string.IsNullOrEmpty(dllPath) || string.IsNullOrEmpty(entry))
            {
                throw new ArgumentException("DLL path and entry is needed");
            }

            if (!File.Exists(dllPath))
            {
                throw new ArgumentException($"DLL doesn't exists in path: {dllPath}");
            }

            strDllPath = dllPath;
            strEntry = entry;
        }

        public void StartDebugger()
        {
            string cmd = $"--app-provider nuget;https://unitetest.chinastock.com.cn:453/v3/index.json --runtime-mode debug " +
                $"--gui-mode hide --debug-app 1 --debugapp-dllpath {strDllPath} --debugapp-entry {strEntry} " +
                $"--network-location public -ext Znjy.Window-```eyJBY2NvdW50IjoiemFuamlhbmh1YSIsIk1kNSI6ImUxMGFkYzM5NDliYTU5YWJiZTU2ZTA1N2YyMGY4ODNlIn0=```eyJBY2NvdW50IjoiMjIyODIiLCJNZDUiOiJlMTBhZGMzOTQ5YmE1OWFiYmU1NmUwNTdmMjBmODgzZSIsIlNwaWRlclVybCI6Imh0dHBzOi8vdW5pdGV0ZXN0LmNoaW5hc3RvY2suY29tLmNuOjgwODEiLCJJcCI6IjIyMy43MC4xMjQuMjI5IiwiUG9ydCI6OTk5OSwiUHJvZHVjdElEIjoiR01hdHJpeCJ9```";
            new WpfCosmosEngineLauncher<WpfVendorWindow>().Launch(cmd.Split(' '));
        }

        private string strDllPath;
        private string strEntry;
    }
}
