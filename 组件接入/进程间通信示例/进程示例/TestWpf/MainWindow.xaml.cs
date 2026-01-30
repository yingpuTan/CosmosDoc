using System;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using RpcWrapper;
using static RpcWrapper.CSharpRpcWrapper;

namespace TestWpf
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr SetParent(IntPtr hWnd, IntPtr hParent);

        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int nWidth, int nHeight, bool uFlags);

        [DllImport("user32.dll")]
        private static extern long GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll")]
        private static extern int SetWindowLong(IntPtr hWnd, int nIndex, long dwNewLong);

        [DllImport("user32.dll")]
        private static extern bool InvalidateRect(IntPtr hWnd, IntPtr lpRect, bool bErase);

        [DllImport("user32.dll")]
        private static extern bool UpdateWindow(IntPtr hWnd);

        const long WS_CHILD         = 0x40000000;
        const long WS_BORDER        = 0x00800000;
        const long WS_POPUP         = 0x80000000;
        const long WS_THICKFRAME    = 0x00040000;
        const long WS_DLGFRAME      = 0x00400000;
        const long WS_EX_WINDOWEDGE = 0x00000100;
        const long WS_EX_CLIENTEDGE = 0x00000200;

        public string _wndInfo { get; set; }
        public string _pipeName { get; set; }
        private bool  _pipeSucc { get; set; }

        private IntPtr curHwnd;

        private string? theme;

        private Dictionary<string, Dictionary<string, object>>? _themeResources;

        static HashSet<string> g_setSub = new HashSet<string>();

        public class AccountInfo
        {
            public string ID { get; set; }
            public int Type { get; set; } = 0;
            public int Status { get; set; } = 0;
        }

        static Dictionary<string, AccountInfo> g_mapActInfo = new Dictionary<string, AccountInfo>();

        private CSharpRpcWrapper wrapper = new CSharpRpcWrapper();

        public MainWindow()
        {
            // 初始化全局变量
            AccountInfo info = new AccountInfo
            {
                ID = "123456"
            };

            g_mapActInfo["123456"] = info;

            InitializeComponent();
            this.Loaded += MainWindow_Loaded;
            this.SourceInitialized += MainWindow_SourceInitialized;

            wrapper.OnInvoke += On_Invoke;
            wrapper.OnPush += On_Push;
            wrapper.OnNotify += On_Notify;
            wrapper.OnSubscribe += On_Subscribe;
        }

        private void RegisterCallBack()
        {
            // 注册回调函数
            wrapper.RegisterCallback();
        }
        private RpcResponse On_Invoke(RpcRequest request)
        {
            RpcResponse resp = new RpcResponse();
            resp.id = request.id;
            resp.code = 0;  //默认设置成功

            string strMethod = (string)request.method;
            if (strMethod == "qry_account")
            {
                // 查询账户
                string id = (string)request.param["ID"];
                if (id == "ALL")
                {
                    // 查询所有账户信息
                    List<AccountInfo> accounts = new List<AccountInfo>
                        {
                            new AccountInfo { ID = "123456", Type = 1, Status = 1 },
                            new AccountInfo { ID = "456789", Type = 2, Status = 0 }
                        };

                    resp.result = JToken.FromObject(accounts);
                }
                else if (id == "123456")
                {
                    // 查询指定账户ID的账户信息（假设 ID 为 "123456"，即返回账户123456的信息）
                    resp.result = new JObject
                    {
                        ["ID"] = "123456",
                        ["Type"] = 1,
                        ["Status"] = 1,
                    };
                }
                else
                {
                    resp.code = -1;
                    resp.error = new JObject
                    {
                        ["msg"] = "未找到查询的账户ID",
                    };
                }
            }
            else
            {
                // 其他查询业务
            }

            return resp;
        }

        private double lastWidth = 0;
        private double lastHeight = 0;
        private double lastSetWidth = 0;
        private double lastSetHeight = 0;
        private double lastScaling = 0;

        private async void UpdateWindow()
        {
            await Task.Delay(500);

            Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {

                MoveWindow(curHwnd, 0, 0, (int)lastSetWidth, (int)lastSetHeight, false);
                InvalidateRect(curHwnd, IntPtr.Zero, true);
                UpdateWindow(curHwnd);

                InvalidateMeasure();
                InvalidateArrange();
                UpdateLayout();
            }));
        }

        private async void On_Notify(RpcRequest request)
        {
            if (request.method == "setsize")
            {
                // 调整尺寸位置
                double scaling = (double)request.param["curscaling"];
                double dpiratio = (double)request.param["dpiRatio"];
                double prescaling = (double)request.param["prescaling"];
                double width = (double)request.param["width"];
                double height = (double)request.param["height"];

                double actwidth = width * dpiratio / scaling;
                double actheight = height * dpiratio / scaling;

                if (lastWidth != width || lastHeight != height && scaling != prescaling || lastScaling == 0)
                {
                    lastWidth = width;
                    lastHeight = height;

                    lastSetWidth = actwidth;
                    lastSetHeight = actheight;
                    lastScaling = scaling;

                    UpdateWindow();
                }
            }
            else if (request.method == "close")
            {
                // 关闭
                Application.Current.Dispatcher.BeginInvoke(new Action(() =>
                {
                    wrapper?.Exit();
                    this.Close();
                }));
            }
            else if(request.method == "SchemeChanged")
            {
                // 主题切换
                Dispatcher.Invoke(() =>
                {
                    theme = (string)request.param["theme"];
                    var converte = new BrushConverter();

                    Dictionary<string, object>? colorDic = _themeResources?[theme];

                    ResourceDictionary resourseDict = new ResourceDictionary();
                    foreach (KeyValuePair<string, object> kvp in colorDic)
                    {
                        resourseDict.Add(kvp.Key, converte.ConvertFrom(kvp.Value));
                    }
                    if (Application.Current.Resources.MergedDictionaries.Count == 0)
                    {
                        Application.Current.Resources.MergedDictionaries.Add(resourseDict);
                    }
                    else
                    {
                        Application.Current.Resources.MergedDictionaries[0] = resourseDict;
                    }
                }
                );
            }
            else
            {
                // 其他通知业务
            }
        }

        private void On_Push(RpcPush push)
        {
            // 根据 topic 判断具体业务
            if (push.topic == "push_account")
            {
                // 推送账户信息
                string strID = (string)push.param["ID"];
                if (g_mapActInfo.ContainsKey(strID))
                {
                    g_mapActInfo[strID].Type = (int)push.param["Type"];
                    g_mapActInfo[strID].Status = (int)push.param["Status"];
                }
            }
            else
            {
                // 其他推送业务
            }
        }

        private void On_Subscribe(RpcRequest request)
        {
            if (request.method == "sub_account")
            {
                // 订阅账户
                if (!string.IsNullOrEmpty((string)request.param["ID"]))
                {
                    // 缓存订阅的key
                    string strSubkey = $"{request.method}_{request.param["ID"]}";
                    g_setSub.Add(strSubkey);
                }
            }
            else
            {
                // 其他订阅业务
            }
        }

        private void MainWindow_SourceInitialized(object sender, EventArgs e)
        {
            this.SourceInitialized -= MainWindow_SourceInitialized;

            if(string.IsNullOrEmpty(_wndInfo))
            {
                return;
            }

            string[] parts = _wndInfo.Split('|');
            if (parts.Length > 2)
            {
                // 设置父子关系和窗口位置
                string msg = parts[0];
                string strwidth = parts[1];
                string strheight = parts[2];

                int parHandle = 0;
                int.TryParse(msg, out parHandle);
                curHwnd = new WindowInteropHelper(this).Handle;
                long lPreStyle = GetWindowLong(curHwnd, -16);
                long lPreExStyle = GetWindowLong(curHwnd, -20);

                lPreStyle &= ~WS_POPUP;
                lPreStyle |= WS_CHILD;
                lPreStyle &= ~(WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);
                lPreExStyle &= ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);
                SetWindowLong(curHwnd, -16, lPreStyle);
                SetWindowLong(curHwnd, -20, lPreExStyle);
                SetParent(curHwnd, parHandle);
                WindowStyle = WindowStyle.None;
                ResizeMode = ResizeMode.NoResize;

                double width = 0;
                double.TryParse(strwidth, out width);
                double height = 0;
                double.TryParse(strheight, out height);
                MoveWindow(curHwnd, 0, 0, (int)width, (int)height, false);
                InvalidateRect(curHwnd, IntPtr.Zero, true);
                UpdateWindow(curHwnd);
            }
        }
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            this.Loaded -= MainWindow_Loaded;

            // 注册回调函数
            RegisterCallBack();

            // 连接管道
            Connect();
        }

        private void Connect()
        {
            string log_path = "test_rpc_wpf.log";   // 管道日志文件
            if (string.IsNullOrEmpty(_pipeName))
                return;

            if (!wrapper.InitClient(_pipeName, log_path, 2))
            {
                MessageBox.Show("连接失败");
                return;
            }

            _pipeSucc = true;

            // 发送初始化消息
            RpcRequest request = new RpcRequest();
            request.id = Guid.NewGuid().ToString();
            request.method = "init_succ";
            //g_mapRequest[request.id] = request;
            wrapper?.Notify(request);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            // 获取触发事件的按钮
            Button button = sender as Button;

            if (button != null)
            {
                switch (button.Name)
                {
                    case "notify":
                        {
                            if (_pipeSucc)
                            {
                                RpcRequest request = new RpcRequest();
                                request.id = Guid.NewGuid().ToString();
                                request.method = "notf_sub";
                                //g_mapRequest[request.id] = request;
                                wrapper?.Notify(request);
                            }
                            else
                                MessageBox.Show("请先建立连接");
                            break;
                        }
                    case "push":
                        {
                            if (_pipeSucc)
                            {
                                // 检查是否订阅了该key
                                string strSubkey = "sub_account_123456";
                                if (g_setSub.Contains(strSubkey))
                                {
                                    // 说明订阅过该key，则可以推送数据
                                    RpcPush push = new RpcPush();
                                    push.topic = "push_account";
                                    push.param = new JObject
                                    {
                                        ["ID"] = "123456",
                                        ["Type"] = 1,
                                        ["Status"] = 1
                                    };
                                    wrapper?.Push(push);
                                }
                            }
                            else
                                MessageBox.Show("请先建立连接");
                            break;
                        }
                    case "invokeAsync":
                        {
                            if (_pipeSucc)
                            {
                                TestMethodInvokeAsync();
                                MessageBox.Show("异步测试后续流程 - 不阻塞");
                            }
                            else
                                MessageBox.Show("请先建立连接");
                            break;
                        }
                    case "invoke":
                        {
                            if (_pipeSucc)
                            {
                                TestMethodInvoke();
                                MessageBox.Show("同步测试后续流程 - 被阻塞");
                            }
                            else
                                MessageBox.Show("请先建立连接");
                            break;
                        }
                    case "requestThemeRes":
                        {
                            // 先获取到颜色资源字典
                            RquestThemeRes();
                            break;
                        }
                    case "requestTheme":
                        {
                            // 获取到颜色资源字典后
                            // 获取主题 根据主题加载对应资源
                            RequestTheme();
                            break;
                        }
                    case "DemoButton":
                        {
                            break;
                        }
                }
            }
        }

        private async Task TestMethodInvokeAsync()
        {
            RpcRequest request = new RpcRequest();
            request.id = Guid.NewGuid().ToString();
            request.method = "test_invoke_async";

            var (ret, response) = await wrapper.InvokeAsync(request);
            if (RET_CALL.Ok == (RET_CALL)ret)
            {
                MessageBox.Show("异步测试结果返回");
            }
        }

        private void TestMethodInvoke()
        {
            RpcRequest request = new RpcRequest();
            request.id = Guid.NewGuid().ToString();
            request.method = "test_invoke";
            RpcResponse response = new RpcResponse();

            int interval = 30000;
            int ret = wrapper.Invoke(request, out response);
            if (RET_CALL.Ok == (RET_CALL)ret)
            {
                MessageBox.Show("同步测试结果返回");
            }
        }
        private void RequestTheme()
        {
            if (wrapper is not null)
            {
                RpcRequest request = new RpcRequest();
                request.method = "requestTheme";
                RpcResponse response = new RpcResponse();
                int ret = wrapper.Invoke(request, out response);
                if (ret == 1 && response.code == 0)
                {
                    var converter = new BrushConverter();

                    theme = response.result["theme"].ToString();
                    Dictionary<string, object>? colorDic = _themeResources?[theme];

                    ResourceDictionary resourseDict = new ResourceDictionary();
                    foreach (KeyValuePair<string, object> kvp in colorDic)
                    {
                        resourseDict.Add(kvp.Key, converter.ConvertFrom(kvp.Value));
                    }
                    if (Application.Current.Resources.MergedDictionaries.Count == 0)
                    {
                        Application.Current.Resources.MergedDictionaries.Add(resourseDict);
                    }
                    else
                    {
                        Application.Current.Resources.MergedDictionaries[0] = resourseDict;
                    }

                    DemoButton.SetResourceReference(Button.BackgroundProperty, "color-background2");
                    DemoButton.SetResourceReference(Button.ForegroundProperty, "color-font5");
                }
            }
        }

        private void RquestThemeRes()
        {
            if(wrapper is not null)
            {
                RpcRequest request = new RpcRequest();
                request.method = "requestThemeRes";
                RpcResponse response = new RpcResponse();
                int ret = wrapper.Invoke(request, out response);
                if (ret == 1 && response.code == 0)
                {
                    string outputMessage = response.result.ToString();
                    _themeResources = Newtonsoft.Json.JsonConvert.DeserializeObject<Dictionary<string, Dictionary<string, object>>>(outputMessage);
                }
            }
        }

        private async void Button_Click_1(object sender, RoutedEventArgs e)
        {
            RpcRequest request = new RpcRequest();
            request.method = "textchanged";
            request.param = new JObject()
            {
                ["text"] = edit_content.Text
            };
            if (comType.SelectedIndex == 0)
            {
                var response = await wrapper.InvokeWidget(edit_group.Text, InvokeType.Global, request, false);
                if ((RET_CALL)response.ret == RET_CALL.Ok)
                {
                    var res = response.response;
                    Console.WriteLine($"{res.result}");
                }
            }
            else
            {
                wrapper.NotifyWidget(edit_group.Text, InvokeType.Global, request, false);

            }
        }

        private async void Button_Click_2(object sender, RoutedEventArgs e)
        {
            RpcRequest request = new RpcRequest();
            request.method = "textchanged";
            request.param = new JObject()
            {
                ["text"] = edit_content.Text
            };
            if (comType.SelectedIndex == 0)
            {
                var response = await wrapper.InvokeWidget(edit_group.Text, InvokeType.Group, request, false);
                if ((RET_CALL)response.ret == RET_CALL.Ok)
                {
                    var res = response.response;
                    Console.WriteLine($"{res.result}");
                }
            }
            else
            {
                wrapper.NotifyWidget(edit_group.Text, InvokeType.Group, request, false);

            }
        }
    }
}