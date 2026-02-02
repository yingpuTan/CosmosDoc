using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;
using Newtonsoft.Json.Linq;
using RpcWrapper;
using static RpcWrapper.CSharpRpcWrapper;

namespace TestAvalonia
{
    public partial class MainWindow : Window
    {
        public string? _wndInfo { get; set; }
        public string? _pipeName { get; set; }
        private bool _pipeSucc { get; set; }

        private string? theme;
        private Dictionary<string, Dictionary<string, object>>? _themeResources;

        static HashSet<string> g_setSub = new HashSet<string>();

        public class AccountInfo
        {
            public string ID { get; set; } = "";
            public int Type { get; set; } = 0;
            public int Status { get; set; } = 0;
        }

        static Dictionary<string, AccountInfo> g_mapActInfo = new Dictionary<string, AccountInfo>();

        private CSharpRpcWrapper wrapper = new CSharpRpcWrapper();

        public MainWindow()
        {
            InitializeComponent();
            
            // 初始化全局变量
            AccountInfo info = new AccountInfo
            {
                ID = "123456"
            };
            g_mapActInfo["123456"] = info;

            this.Opened += MainWindow_Opened;

            wrapper.OnInvoke += On_Invoke;
            wrapper.OnPush += On_Push;
            wrapper.OnNotify += On_Notify;
            wrapper.OnSubscribe += On_Subscribe;
        }

        private void MainWindow_Opened(object? sender, EventArgs e)
        {
            // 注册回调函数
            RegisterCallBack();

            // 连接管道（如果提供了管道名称）
            if (!string.IsNullOrEmpty(_pipeName))
            {
                Connect();
            }
            else
            {
                // 显示连接对话框
                ShowConnectDialog();
            }
        }

        private async void ShowConnectDialog()
        {
            var dialog = new Window
            {
                Title = "连接 RPC 管道",
                Width = 400,
                Height = 150,
                WindowStartupLocation = WindowStartupLocation.CenterOwner
            };

            var panel = new StackPanel
            {
                Margin = new Thickness(20),
                Spacing = 10
            };

            var textBlock = new TextBlock { Text = "请输入管道名称:" };
            var textBox = new TextBox { Watermark = "pipe_name" };
            var buttonPanel = new StackPanel
            {
                Orientation = Avalonia.Layout.Orientation.Horizontal,
                HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Right,
                Spacing = 10
            };

            var okButton = new Button { Content = "确定", Width = 80 };
            var cancelButton = new Button { Content = "取消", Width = 80 };

            okButton.Click += (s, e) =>
            {
                _pipeName = textBox.Text;
                if (!string.IsNullOrEmpty(_pipeName))
                {
                    Connect();
                    dialog.Close();
                }
            };

            cancelButton.Click += (s, e) => dialog.Close();

            buttonPanel.Children.Add(okButton);
            buttonPanel.Children.Add(cancelButton);

            panel.Children.Add(textBlock);
            panel.Children.Add(textBox);
            panel.Children.Add(buttonPanel);

            dialog.Content = panel;
            await dialog.ShowDialog(this);
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
            resp.code = 0;  // 默认设置成功

            string strMethod = request.method ?? "";
            AddStatusMessage($"[回调] 收到 Invoke 请求: {strMethod}");

            if (strMethod == "qry_account")
            {
                // 查询账户
                string id = request.param?["ID"]?.ToString() ?? "";
                if (id == "ALL")
                {
                    // 查询所有账户信息
                    List<AccountInfo> accounts = new List<AccountInfo>
                    {
                        new AccountInfo { ID = "123456", Type = 1, Status = 1 },
                        new AccountInfo { ID = "456789", Type = 2, Status = 0 }
                    };

                    resp.result = JToken.FromObject(accounts);
                    AddStatusMessage($"  → 返回所有账户信息，共 {accounts.Count} 个账户");
                }
                else if (id == "123456")
                {
                    // 查询指定账户ID的账户信息
                    resp.result = new JObject
                    {
                        ["ID"] = "123456",
                        ["Type"] = 1,
                        ["Status"] = 1,
                    };
                    AddStatusMessage($"  → 返回账户 {id} 的信息");
                }
                else
                {
                    resp.code = -1;
                    resp.error = new JObject
                    {
                        ["msg"] = "未找到查询的账户ID",
                    };
                    AddStatusMessage($"  → 账户 {id} 不存在");
                }
            }
            else
            {
                AddStatusMessage($"  → 处理其他业务: {strMethod}");
            }

            return resp;
        }

        private void On_Notify(RpcRequest request)
        {
            string method = request.method ?? "";
            AddStatusMessage($"[回调] 收到 Notify: {method}");

            if (method == "setsize")
            {
                // 调整尺寸位置（Avalonia 中不需要 Win32 API）
                double scaling = request.param?["curscaling"]?.ToObject<double>() ?? 1.0;
                double dpiratio = request.param?["dpiRatio"]?.ToObject<double>() ?? 1.0;
                double width = request.param?["width"]?.ToObject<double>() ?? 0;
                double height = request.param?["height"]?.ToObject<double>() ?? 0;

                Dispatcher.UIThread.Post(() =>
                {
                    this.Width = width;
                    this.Height = height;
                });

                AddStatusMessage($"  → 收到窗口尺寸调整通知: {width}x{height}");
            }
            else if (method == "close")
            {
                // 关闭
                Dispatcher.UIThread.Post(() =>
                {
                    wrapper?.Exit();
                    this.Close();
                });
                AddStatusMessage("  → 收到关闭通知");
            }
            else if (method == "SchemeChanged")
            {
                // 主题切换
                Dispatcher.UIThread.Post(() =>
                {
                    theme = request.param?["theme"]?.ToString();
                    ApplyTheme(theme);
                });
                AddStatusMessage($"  → 收到主题切换通知: {theme}");
            }
            else
            {
                AddStatusMessage($"  → 其他通知业务: {method}");
            }
        }

        private void On_Push(RpcPush push)
        {
            string topic = push.topic ?? "";
            AddStatusMessage($"[回调] 收到 Push, Topic: {topic}");

            if (topic == "push_account")
            {
                // 推送账户信息
                string strID = push.param?["ID"]?.ToString() ?? "";
                if (g_mapActInfo.ContainsKey(strID))
                {
                    g_mapActInfo[strID].Type = push.param?["Type"]?.ToObject<int>() ?? 0;
                    g_mapActInfo[strID].Status = push.param?["Status"]?.ToObject<int>() ?? 0;
                    AddStatusMessage($"  → 更新账户 {strID} 信息");
                }
            }
            else
            {
                AddStatusMessage($"  → 其他推送业务: {topic}");
            }
        }

        private void On_Subscribe(RpcRequest request)
        {
            string method = request.method ?? "";
            AddStatusMessage($"[回调] 收到 Subscribe: {method}");

            if (method == "sub_account")
            {
                // 订阅账户
                string id = request.param?["ID"]?.ToString() ?? "";
                if (!string.IsNullOrEmpty(id))
                {
                    // 缓存订阅的key
                    string strSubkey = $"{method}_{id}";
                    g_setSub.Add(strSubkey);
                    AddStatusMessage($"  → 订阅账户 {id}");
                }
            }
            else
            {
                AddStatusMessage($"  → 其他订阅业务: {method}");
            }
        }

        private void Connect()
        {
            string log_path = "test_rpc_avalonia.log";   // 管道日志文件
            if (string.IsNullOrEmpty(_pipeName))
            {
                AddStatusMessage("管道名称未设置！");
                return;
            }

            if (!wrapper.InitClient(_pipeName, log_path, 2))
            {
                AddStatusMessage("连接失败！");
                ShowMessage("连接失败", "无法连接到 RPC 管道");
                return;
            }

            _pipeSucc = true;
            AddStatusMessage($"✓ 连接成功: {_pipeName}");

            // 发送初始化消息
            RpcRequest request = new RpcRequest();
            request.id = Guid.NewGuid().ToString();
            request.method = "init_succ";
            wrapper?.Notify(request);
        }

        private void Button_Click(object? sender, RoutedEventArgs e)
        {
            // 获取触发事件的按钮
            Button? button = sender as Button;

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
                                wrapper?.Notify(request);
                                AddStatusMessage("✓ Notify 发送成功");
                            }
                            else
                                ShowMessage("提示", "请先建立连接");
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
                                    AddStatusMessage("✓ Push 发送成功");
                                }
                                else
                                {
                                    ShowMessage("提示", "请先订阅账户");
                                }
                            }
                            else
                                ShowMessage("提示", "请先建立连接");
                            break;
                        }
                    case "invokeAsync":
                        {
                            if (_pipeSucc)
                            {
                                _ = TestMethodInvokeAsync();
                                AddStatusMessage("异步测试已启动，不阻塞界面");
                            }
                            else
                                ShowMessage("提示", "请先建立连接");
                            break;
                        }
                    case "invoke":
                        {
                            if (_pipeSucc)
                            {
                                TestMethodInvoke();
                                AddStatusMessage("同步测试完成");
                            }
                            else
                                ShowMessage("提示", "请先建立连接");
                            break;
                        }
                    case "requestThemeRes":
                        {
                            // 先获取到颜色资源字典
                            RequestThemeRes();
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
                AddStatusMessage("✓ 异步 Invoke 成功");
                AddStatusMessage($"  响应 ID: {response.id}");
                AddStatusMessage($"  响应码: {response.code}");
            }
            else
            {
                AddStatusMessage($"✗ 异步 Invoke 失败，返回码: {ret}");
            }
        }

        private void TestMethodInvoke()
        {
            RpcRequest request = new RpcRequest();
            request.id = Guid.NewGuid().ToString();
            request.method = "test_invoke";
            RpcResponse response = new RpcResponse();

            int ret = wrapper.Invoke(request, out response, 30000);
            if (RET_CALL.Ok == (RET_CALL)ret)
            {
                AddStatusMessage("✓ 同步 Invoke 成功");
                AddStatusMessage($"  响应 ID: {response.id}");
                AddStatusMessage($"  响应码: {response.code}");
            }
            else
            {
                AddStatusMessage($"✗ 同步 Invoke 失败，返回码: {ret}");
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
                    theme = response.result?["theme"]?.ToString();
                    ApplyTheme(theme);
                    AddStatusMessage($"✓ 获取主题成功: {theme}");
                }
                else
                {
                    AddStatusMessage($"✗ 获取主题失败");
                }
            }
        }

        private void RequestThemeRes()
        {
            if (wrapper is not null)
            {
                RpcRequest request = new RpcRequest();
                request.method = "requestThemeRes";
                RpcResponse response = new RpcResponse();
                int ret = wrapper.Invoke(request, out response);
                if (ret == 1 && response.code == 0)
                {
                    string outputMessage = response.result?.ToString() ?? "";
                    _themeResources = Newtonsoft.Json.JsonConvert.DeserializeObject<Dictionary<string, Dictionary<string, object>>>(outputMessage);
                    AddStatusMessage("✓ 获取主题资源成功");
                }
                else
                {
                    AddStatusMessage("✗ 获取主题资源失败");
                }
            }
        }

        private void ApplyTheme(string? themeName)
        {
            if (string.IsNullOrEmpty(themeName) || _themeResources == null)
                return;

            if (!_themeResources.ContainsKey(themeName))
                return;

            var colorDic = _themeResources[themeName];
            // 在 Avalonia 中应用主题资源
            // 这里可以根据需要实现主题切换逻辑
            AddStatusMessage($"应用主题: {themeName}");
        }

        private async void Button_Click_1(object? sender, RoutedEventArgs e)
        {
            RpcRequest request = new RpcRequest();
            request.method = "textchanged";
            request.param = new JObject()
            {
                ["text"] = edit_content.Text ?? ""
            };
            
            if (comType.SelectedIndex == 0)
            {
                var response = await wrapper.InvokeWidget(edit_group.Text ?? "", InvokeType.Global, request, false);
                if ((RET_CALL)response.ret == RET_CALL.Ok)
                {
                    var res = response.response;
                    AddStatusMessage($"全局请求成功: {res.result}");
                }
            }
            else
            {
                wrapper.NotifyWidget(edit_group.Text ?? "", InvokeType.Global, request, false);
                AddStatusMessage("全局通知已发送");
            }
        }

        private async void Button_Click_2(object? sender, RoutedEventArgs e)
        {
            RpcRequest request = new RpcRequest();
            request.method = "textchanged";
            request.param = new JObject()
            {
                ["text"] = edit_content.Text ?? ""
            };
            
            if (comType.SelectedIndex == 0)
            {
                var response = await wrapper.InvokeWidget(edit_group.Text ?? "", InvokeType.Group, request, false);
                if ((RET_CALL)response.ret == RET_CALL.Ok)
                {
                    var res = response.response;
                    AddStatusMessage($"组请求成功: {res.result}");
                }
            }
            else
            {
                wrapper.NotifyWidget(edit_group.Text ?? "", InvokeType.Group, request, false);
                AddStatusMessage("组通知已发送");
            }
        }

        private void AddStatusMessage(string message)
        {
            Dispatcher.UIThread.Post(() =>
            {
                if (statusText != null)
                {
                    statusText.Text += $"{DateTime.Now:HH:mm:ss} - {message}\n";
                    // 自动滚动到底部
                    var scrollViewer = statusText.Parent as ScrollViewer;
                    scrollViewer?.ScrollToEnd();
                }
            });
        }

        private async void ShowMessage(string title, string message)
        {
            var dialog = new Window
            {
                Title = title,
                Width = 300,
                Height = 150,
                WindowStartupLocation = WindowStartupLocation.CenterOwner
            };

            var panel = new StackPanel
            {
                Margin = new Thickness(20),
                Spacing = 10
            };

            var textBlock = new TextBlock 
            { 
                Text = message, 
                TextWrapping = TextWrapping.Wrap 
            };

            var buttonPanel = new StackPanel
            {
                Orientation = Avalonia.Layout.Orientation.Horizontal,
                HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Right,
                Margin = new Thickness(0, 10, 0, 0)
            };

            var okButton = new Button 
            { 
                Content = "确定", 
                Width = 80 
            };

            okButton.Click += (s, e) => dialog.Close();

            buttonPanel.Children.Add(okButton);
            panel.Children.Add(textBlock);
            panel.Children.Add(buttonPanel);
            dialog.Content = panel;

            await dialog.ShowDialog(this);
        }

        protected override void OnClosed(EventArgs e)
        {
            wrapper?.Exit();
            base.OnClosed(e);
        }
    }
}

