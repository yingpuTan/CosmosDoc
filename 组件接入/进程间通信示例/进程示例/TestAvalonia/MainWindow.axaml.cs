using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Threading;
using Avalonia.Platform;
using System.Reflection;
using System.Runtime.InteropServices;
using Newtonsoft.Json.Linq;
using RpcWrapper;
using static RpcWrapper.CSharpRpcWrapper;

#if WINDOWS
using Avalonia.Win32;
#endif

namespace TestAvalonia
{
    public partial class MainWindow : Window
    {
        public string? _wndInfo { get; set; }
        public string? _pipeName { get; set; }
        private bool _pipeSucc { get; set; }

        private IntPtr curHwnd = IntPtr.Zero;
        private ulong curX11Window = 0;  // Linux X11 窗口 ID

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

#if WINDOWS
        // Windows 平台 Win32 API
        [DllImport("user32.dll", SetLastError = true)]
        private static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);

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

        [DllImport("user32.dll")]
        private static extern IntPtr GetParent(IntPtr hWnd);

        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

        [DllImport("user32.dll")]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        [DllImport("user32.dll")]
        private static extern IntPtr PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern bool IsWindow(IntPtr hWnd);

        const uint SWP_NOZORDER = 0x0004;
        const uint SWP_NOACTIVATE = 0x0010;
        const uint SWP_SHOWWINDOW = 0x0040;
        const int SW_HIDE = 0;
        const int SW_SHOW = 5;

        const long WS_CHILD = 0x40000000;
        const long WS_BORDER = 0x00800000;
        const long WS_POPUP = 0x80000000;
        const long WS_THICKFRAME = 0x00040000;
        const long WS_DLGFRAME = 0x00400000;
        const long WS_EX_WINDOWEDGE = 0x00000100;
        const long WS_EX_CLIENTEDGE = 0x00000200;
#elif LINUX
        // Linux 平台 X11 API
        [DllImport("libX11.so.6")]
        private static extern IntPtr XOpenDisplay(IntPtr display);

        [DllImport("libX11.so.6")]
        private static extern int XCloseDisplay(IntPtr display);

        [DllImport("libX11.so.6")]
        private static extern int XReparentWindow(IntPtr display, ulong w, ulong parent, int x, int y);

        [DllImport("libX11.so.6")]
        private static extern int XMoveResizeWindow(IntPtr display, ulong w, int x, int y, int width, int height);

        [DllImport("libX11.so.6")]
        private static extern int XMapRaised(IntPtr display, ulong w);

        [DllImport("libX11.so.6")]
        private static extern int XFlush(IntPtr display);

        [DllImport("libX11.so.6")]
        private static extern ulong XDefaultRootWindow(IntPtr display);

        [DllImport("libX11.so.6")]
        private static extern int XSetWindowBorderWidth(IntPtr display, ulong w, uint width);

        [DllImport("libX11.so.6")]
        private static extern int XUnmapWindow(IntPtr display, ulong w);

        [DllImport("libX11.so.6")]
        private static extern int XMapWindow(IntPtr display, ulong w);

        private static IntPtr? x11Display = null;
#endif

        public MainWindow()
        {
            InitializeComponent();
            
            // 初始化全局变量
            AccountInfo info = new AccountInfo
            {
                ID = "123456"
            };
            g_mapActInfo["123456"] = info;

            // 使用 Opened 事件处理连接
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

            // 延迟处理窗口嵌入，确保窗口句柄已创建
            if (!string.IsNullOrEmpty(_wndInfo))
            {
                // 使用多个时机尝试设置父窗口，确保窗口句柄已创建
                SetupParentWindowWithRetry();
            }
        }

        private void SetupParentWindow()
        {
            if (string.IsNullOrEmpty(_wndInfo))
            {
                return;
            }

            string[] parts = _wndInfo.Split('|');
            if (parts.Length < 3)
            {
                return;
            }

            // 解析窗口信息：parentHandle|width|height
            string msg = parts[0];
            string strwidth = parts[1];
            string strheight = parts[2];

            int parHandle = 0;
            int.TryParse(msg, out parHandle);

            if (parHandle == 0)
            {
                return;
            }

#if WINDOWS
            // Windows 平台：使用 Win32 API 设置父窗口
            TrySetupParentWindow();
#elif LINUX
            // Linux 平台：使用 X11 API 设置父窗口
            SetupParentWindowLinux(parHandle, strwidth, strheight);
#else
            // macOS 平台：暂不支持
            AddStatusMessage("macOS 平台的窗口嵌入功能需要额外实现");
#endif
        }

#if WINDOWS
        private void SetupParentWindowWithRetry(int retryCount = 0)
        {
            const int maxRetries = 10;
            const int retryDelay = 50; // 毫秒

            if (retryCount >= maxRetries)
            {
                AddStatusMessage("设置父窗口失败：超过最大重试次数");
                return;
            }

            Dispatcher.UIThread.Post(() =>
            {
                // 尝试获取窗口句柄
                curHwnd = GetWindowHandle();
                if (curHwnd == IntPtr.Zero)
                {
                    // 如果获取失败，延迟重试
                    Task.Delay(retryDelay).ContinueWith(_ =>
                    {
                        SetupParentWindowWithRetry(retryCount + 1);
                    });
                    return;
                }

                // 窗口句柄已获取，尝试设置父窗口
                TrySetupParentWindow();
            }, DispatcherPriority.Loaded);
        }
#endif

#if WINDOWS
        private void TrySetupParentWindow()
        {
            if (string.IsNullOrEmpty(_wndInfo))
            {
                return;
            }

            string[] parts = _wndInfo.Split('|');
            if (parts.Length < 3)
            {
                return;
            }

            string msg = parts[0];
            string strwidth = parts[1];
            string strheight = parts[2];

            int parHandle = 0;
            int.TryParse(msg, out parHandle);

            if (parHandle == 0)
            {
                return;
            }

            try
            {
                // 尝试获取当前窗口句柄（可能需要多次尝试）
                curHwnd = GetWindowHandle();
                if (curHwnd == IntPtr.Zero)
                {
                    // 如果获取失败，延迟重试
                    Dispatcher.UIThread.Post(() =>
                    {
                        Task.Delay(50).ContinueWith(_ =>
                        {
                            Dispatcher.UIThread.Post(() => TrySetupParentWindow(), DispatcherPriority.Loaded);
                        });
                    }, DispatcherPriority.Loaded);
                    return;
                }

                // 验证父窗口句柄是否有效
                IntPtr parentHwnd = new IntPtr(parHandle);
                if (parentHwnd == IntPtr.Zero)
                {
                    AddStatusMessage("父窗口句柄无效");
                    return;
                }

                // 解析窗口大小
                double width = 0;
                double.TryParse(strwidth, out width);
                double height = 0;
                double.TryParse(strheight, out height);

                // 验证父窗口句柄是否有效（检查窗口是否存在）
                if (!IsWindow(parentHwnd))
                {
                    AddStatusMessage($"父窗口句柄无效或窗口不存在: {parentHwnd.ToInt64()}");
                    return;
                }

                // 先隐藏窗口（如果可见）- 这很重要，可以避免窗口状态冲突
                ShowWindow(curHwnd, SW_HIDE);

                // 设置窗口样式（必须在 SetParent 之前）
                long lPreStyle = GetWindowLong(curHwnd, -16);  // GWL_STYLE
                long lPreExStyle = GetWindowLong(curHwnd, -20); // GWL_EXSTYLE

                // 保存原始样式（用于调试）
                long originalStyle = lPreStyle;
                long originalExStyle = lPreExStyle;

                lPreStyle &= ~WS_POPUP;
                lPreStyle |= WS_CHILD;
                lPreStyle &= ~(WS_BORDER | WS_THICKFRAME | WS_DLGFRAME);
                lPreExStyle &= ~(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE);

                SetWindowLong(curHwnd, -16, lPreStyle);
                SetWindowLong(curHwnd, -20, lPreExStyle);

                // 设置窗口样式属性（在 SetParent 之前设置，避免窗口管理器干扰）
                this.WindowState = WindowState.Normal;
                this.CanResize = false;
                this.ShowInTaskbar = false;

                // 设置父窗口（关键步骤）
                // 注意：SetParent 的参数类型，TestWpf 中使用的是 int，这里使用 IntPtr
                IntPtr oldParent = SetParent(curHwnd, parentHwnd);
                
                // 清除错误状态
                Marshal.GetLastWin32Error();
                
                // 立即验证父窗口是否设置成功
                IntPtr verifyParent = GetParent(curHwnd);
                if (verifyParent != parentHwnd)
                {
                    AddStatusMessage($"第一次 SetParent 后验证失败。当前父窗口: {verifyParent.ToInt64()}, 期望: {parentHwnd.ToInt64()}");
                    
                    // 再次设置窗口样式，确保样式正确
                    SetWindowLong(curHwnd, -16, lPreStyle);
                    SetWindowLong(curHwnd, -20, lPreExStyle);
                    
                    // 尝试再次设置父窗口
                    IntPtr retryParent = SetParent(curHwnd, parentHwnd);
                    Marshal.GetLastWin32Error(); // 清除错误状态
                    
                    // 再次验证
                    verifyParent = GetParent(curHwnd);
                    if (verifyParent != parentHwnd)
                    {
                        AddStatusMessage($"重试 SetParent 后仍然失败。当前父窗口: {verifyParent.ToInt64()}, 期望: {parentHwnd.ToInt64()}");
                        AddStatusMessage($"原始样式: {originalStyle}, 新样式: {lPreStyle}");
                        ShowWindow(curHwnd, SW_SHOW); // 恢复显示
                        return;
                    }
                    else
                    {
                        AddStatusMessage("重试 SetParent 成功");
                    }
                }

                // 设置窗口样式属性
                this.WindowState = WindowState.Normal;
                this.CanResize = false;
                this.ShowInTaskbar = false;

                // 设置窗口大小和位置
                if (width > 0 && height > 0)
                {
                    this.Width = width;
                    this.Height = height;
                    
                    // 使用 SetWindowPos 强制更新窗口位置和大小
                    SetWindowPos(curHwnd, IntPtr.Zero, 0, 0, (int)width, (int)height, 
                        SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                    
                    // 也调用 MoveWindow 确保位置正确
                    MoveWindow(curHwnd, 0, 0, (int)width, (int)height, false);
                    
                    // 强制刷新窗口
                    InvalidateRect(curHwnd, IntPtr.Zero, true);
                    UpdateWindow(curHwnd);
                }
                else
                {
                    // 即使没有指定大小，也要显示窗口
                    ShowWindow(curHwnd, SW_SHOW);
                }

                // 强制刷新窗口消息队列（使用 Win32 API）
                // 发送 WM_NULL 消息来刷新消息队列
                PostMessage(curHwnd, 0x0000, IntPtr.Zero, IntPtr.Zero);

                // 延迟验证父窗口设置（给系统时间处理窗口消息）
                Task.Delay(100).ContinueWith(_ =>
                {
                    Dispatcher.UIThread.Post(() =>
                    {
                        IntPtr currentParent = GetParent(curHwnd);
                        if (currentParent == parentHwnd)
                        {
                            AddStatusMessage($"窗口已成功嵌入父窗口，句柄: {parHandle}, 大小: {width}x{height}");
                        }
                        else
                        {
                            AddStatusMessage($"警告：父窗口设置可能未生效。当前父窗口: {currentParent.ToInt64()}, 期望: {parentHwnd.ToInt64()}");
                            // 尝试再次设置
                            IntPtr retryParent = SetParent(curHwnd, parentHwnd);
                            if (retryParent != IntPtr.Zero)
                            {
                                SetWindowPos(curHwnd, IntPtr.Zero, 0, 0, (int)width, (int)height, 
                                    SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                                AddStatusMessage("已重试设置父窗口");
                            }
                        }
                    });
                });
            }
            catch (Exception ex)
            {
                AddStatusMessage($"设置父窗口失败: {ex.Message}");
            }
        }
#endif

#if LINUX
        private void SetupParentWindowLinux(int parHandle, string strwidth, string strheight)
        {
            // Linux 平台：使用 X11 API 设置父窗口
            Dispatcher.UIThread.Post(() =>
            {
                try
                {
                    // 获取当前窗口的 X11 窗口 ID
                    curX11Window = GetX11WindowHandle();
                    if (curX11Window == 0)
                    {
                        AddStatusMessage("无法获取 X11 窗口 ID");
                        return;
                    }

                    // 打开 X11 显示连接
                    if (x11Display == null)
                    {
                        x11Display = XOpenDisplay(IntPtr.Zero);
                        if (x11Display == IntPtr.Zero)
                        {
                            AddStatusMessage("无法打开 X11 显示连接");
                            return;
                        }
                    }

                    // 解析窗口大小
                    double width = 0;
                    double.TryParse(strwidth, out width);
                    double height = 0;
                    double.TryParse(strheight, out height);

                    // 将父窗口句柄转换为 ulong（X11 窗口 ID）
                    ulong parentWindowId = (ulong)parHandle;

                    // 设置窗口无边框
                    XSetWindowBorderWidth(x11Display.Value, curX11Window, 0);

                    // 重新设置父窗口
                    int result = XReparentWindow(x11Display.Value, curX11Window, parentWindowId, 0, 0);
                    if (result == 0)
                    {
                        // 设置窗口大小和位置
                        if (width > 0 && height > 0)
                        {
                            XMoveResizeWindow(x11Display.Value, curX11Window, 0, 0, (int)width, (int)height);
                            this.Width = width;
                            this.Height = height;
                        }

                        // 显示窗口
                        XMapRaised(x11Display.Value, curX11Window);
                        XFlush(x11Display.Value);

                        // 设置窗口样式属性
                        this.WindowState = WindowState.Normal;
                        this.CanResize = false;
                        this.ShowInTaskbar = false;

                        AddStatusMessage($"窗口已嵌入父窗口 (X11 ID: {parentWindowId}), 大小: {width}x{height}");
                    }
                    else
                    {
                        AddStatusMessage($"XReparentWindow 失败，返回码: {result}");
                    }
                }
                catch (Exception ex)
                {
                    AddStatusMessage($"设置父窗口失败: {ex.Message}");
                }
            }, DispatcherPriority.Loaded);
        }
#endif

        private IntPtr GetWindowHandle()
        {
#if WINDOWS
            // 获取 Avalonia 窗口句柄（Windows 平台）
            try
            {
                // 方法1: 使用 TryGetPlatformHandle 扩展方法（推荐方式）
                var platformHandle = this.TryGetPlatformHandle();
                if (platformHandle != null)
                {
                    return platformHandle.Handle;
                }

                // 方法2: 尝试通过 PlatformImpl 获取
                if (this.PlatformImpl != null)
                {
                    // 使用反射获取 Handle 属性
                    var handleProperty = this.PlatformImpl.GetType().GetProperty("Handle",
                        BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
                    if (handleProperty != null)
                    {
                        var handle = handleProperty.GetValue(this.PlatformImpl);
                        if (handle is IPlatformHandle platformHandle2)
                        {
                            return platformHandle2.Handle;
                        }
                        else if (handle is IntPtr ptr && ptr != IntPtr.Zero)
                        {
                            return ptr;
                        }
                    }
                }

                // 方法3: 尝试通过反射获取 WindowImpl 的 Handle
                if (this.PlatformImpl != null)
                {
                    var platformImplType = this.PlatformImpl.GetType();
                    // 检查是否是 Windows 平台的实现
                    if (platformImplType.FullName?.Contains("Win32") == true || 
                        platformImplType.FullName?.Contains("Windows") == true)
                    {
                        var handleProperty = platformImplType.GetProperty("Handle",
                            BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
                        if (handleProperty != null)
                        {
                            var handle = handleProperty.GetValue(this.PlatformImpl);
                            if (handle is IPlatformHandle platformHandle3)
                            {
                                return platformHandle3.Handle;
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                AddStatusMessage($"获取窗口句柄时出错: {ex.Message}");
            }
#endif
            return IntPtr.Zero;
        }

#if LINUX
        private ulong GetX11WindowHandle()
        {
            // 获取 Avalonia 窗口的 X11 窗口 ID
            try
            {
                // 方法1: 尝试通过 PlatformImpl 获取
                if (this.PlatformImpl != null)
                {
                    // 使用反射获取窗口句柄
                    var handleProperty = this.PlatformImpl.GetType().GetProperty("Handle",
                        BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance);
                    if (handleProperty != null)
                    {
                        var handle = handleProperty.GetValue(this.PlatformImpl);
                        if (handle is IPlatformHandle platformHandle)
                        {
                            // X11 窗口 ID 是 Handle 的值（ulong）
                            return (ulong)platformHandle.Handle.ToInt64();
                        }
                        else if (handle is IntPtr ptr && ptr != IntPtr.Zero)
                        {
                            return (ulong)ptr.ToInt64();
                        }
                    }
                }

                // 方法2: 尝试通过扩展方法获取
                var platformHandle2 = this.TryGetPlatformHandle();
                if (platformHandle2 != null)
                {
                    return (ulong)platformHandle2.Handle.ToInt64();
                }
            }
            catch (Exception ex)
            {
                AddStatusMessage($"获取 X11 窗口 ID 失败: {ex.Message}");
            }
            return 0;
        }
#endif

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
                // 调整尺寸位置
                double scaling = request.param?["curscaling"]?.ToObject<double>() ?? 1.0;
                double dpiratio = request.param?["dpiRatio"]?.ToObject<double>() ?? 1.0;
                double prescaling = request.param?["prescaling"]?.ToObject<double>() ?? 1.0;
                double width = request.param?["width"]?.ToObject<double>() ?? 0;
                double height = request.param?["height"]?.ToObject<double>() ?? 0;

                double actwidth = width * dpiratio / scaling;
                double actheight = height * dpiratio / scaling;

                Dispatcher.UIThread.Post(() =>
                {
                    if (width > 0 && height > 0)
                    {
                        this.Width = actwidth;
                        this.Height = actheight;

#if WINDOWS
                        // 如果窗口已嵌入父窗口，使用 Win32 API 更新位置和大小
                        if (curHwnd != IntPtr.Zero)
                        {
                            MoveWindow(curHwnd, 0, 0, (int)actwidth, (int)actheight, false);
                            InvalidateRect(curHwnd, IntPtr.Zero, true);
                            UpdateWindow(curHwnd);
                        }
#elif LINUX
                        // 如果窗口已嵌入父窗口，使用 X11 API 更新位置和大小
                        if (curX11Window != 0 && x11Display != null && x11Display != IntPtr.Zero)
                        {
                            XMoveResizeWindow(x11Display.Value, curX11Window, 0, 0, (int)actwidth, (int)actheight);
                            XFlush(x11Display.Value);
                        }
#endif
                    }
                });

                AddStatusMessage($"  → 收到窗口尺寸调整通知: {actwidth}x{actheight} (原始: {width}x{height})");
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
#if LINUX
            // 关闭 X11 显示连接
            if (x11Display != null && x11Display != IntPtr.Zero)
            {
                XCloseDisplay(x11Display.Value);
                x11Display = null;
            }
#endif
            wrapper?.Exit();
            base.OnClosed(e);
        }
    }
}

