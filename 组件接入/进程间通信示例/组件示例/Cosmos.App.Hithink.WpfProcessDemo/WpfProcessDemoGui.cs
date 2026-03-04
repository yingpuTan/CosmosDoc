using System;
using System.Drawing;
using System.Windows;
using System.Windows.Shapes;
using Cosmos.App.Sdk.v1;
using Cosmos.App.Sdk.Windows;
using System.Reflection;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Path = System.IO.Path;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Text.Json.Nodes;
using System.Text.Json;
using System.Windows.Media;
using System.ComponentModel;
using Microsoft.Win32;
using Cosmos.App.Hithink.ProcessDemo.Shared;

namespace Cosmos.App.Hithink.WpfProcessDemo
{
    public class WpfProcessDemoGui :
        WpfCosmosAppProcessWidget //进程间通讯基类，并且需要实现类中提供的抽象方法
    {
        private ProcessDemoGuiBase _baseImpl;

        /// <summary>
        /// 日志记录
        /// </summary>
        protected ICosmosAppLogger _logger { get; set; }

        public WpfProcessDemoGui()
        {
            _wfh = new WpfCosmosHwndHost();
            _wfh.Margin = new Thickness(0, 0, 0, 0);
            /// 设置大小改变事件
            _wfh.SizeChanged += _wfh_SizeChanged;
            SystemParameters.StaticPropertyChanged += SystemParameters_StaticPropertyChanged;

            /// 设置窗口加载成功事件
            _wfh.Loaded += _wfh_Loaded;
            Content = _wfh;

            _baseImpl = new ProcessDemoGuiBase();
            InitializeBaseImpl();
        }

        private void InitializeBaseImpl()
        {
            _baseImpl._widgetInstance = this;
            _baseImpl.GetWindowHandle = () => _wfh.Handle;
            _baseImpl.GetPhysicalWidth = () => _wfh.PhysicalWidth;
            _baseImpl.GetPhysicalHeight = () => _wfh.PhysicalHeight;
            _baseImpl.GetCurrentDpiScale = () => VisualTreeHelper.GetDpi(this).DpiScaleX;
            _baseImpl.GetPreviousDpiScale = () => _previousDpi.DpiScaleX;
            _baseImpl.SetPreviousDpiScale = (scale) => _previousDpi = new DpiScale(scale, scale);
            _baseImpl.SetFirstDpiRatio = (ratio) => _dpiRatioFirst = ratio;
            _baseImpl.GetFirstDpiRatio = () => _dpiRatioFirst;
            _baseImpl.InvokeOnMainThread = (action) => Dispatcher.InvokeAsync(action);
            _baseImpl.CreateRpcRequest = () => CreateRpcRequest();
            _baseImpl.CreateRpcResponse = () => CreateRpcResponse();
            _baseImpl.CreateRpcManager = (clientDir, strCmd, logger) => CreateRpcManager(clientDir, strCmd, logger);
            _baseImpl.RpcNotifyAsync = (request) => RpcNotifyAsync(request);
            _baseImpl.RpcInvokeAsync = (request, timeout) => RpcInvokeAsync(request, timeout);
            _baseImpl.RpcSubscribeAsync = (request) => RpcSubscribeAsync(request);
            _baseImpl.ChangeProcessWindowSize = () => ChangeProcessWindowSize();
        }

        WpfCosmosHwndHost _wfh { get; set; }
        private void _wfh_Loaded(object sender, RoutedEventArgs e)
        {
            _logger?.Log(CosmosLogLevel.Information, "Wfh_Loaded");

            // 获取进程所在路径
            var clientDir = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content\dependents\TestWpf.exe");
            //var clientDir = @"D:\git\itrader\Cosmos_yinhe\Source\RpcClient\TestWpf\bin\Debug\net7.0-windows\win-x64\TestWpf.exe";

            //窗口创建成功、可以启动进程通信服务
            _baseImpl.StartRpcServer(clientDir);
        }

        private void _wfh_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            ChangeProcessWindowSize();
        }

        private void SystemParameters_StaticPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            var currentDpi = VisualTreeHelper.GetDpi(this);
            _logger.Log(CosmosLogLevel.Information, "SystemParameters_StaticPropertyChanged");
            if (currentDpi.DpiScaleX != _previousDpi.DpiScaleX)
            {
                _previousDpi = currentDpi;
                ChangeProcessWindowSize();
            }
        }

        private void ChangeProcessWindowSize()
        {
            try
            {
                //切换到主线程获取窗口大小
                Dispatcher.InvokeAsync(() =>
                {
                    //窗口发生变化、通知进程需要调整窗口大小
                    //组装调整大小命令、进程按照该组装方式进行解析
                    var graphics = VisualTreeHelper.GetDpi(this);
                    ICosmosRpcRequest request = CreateRpcRequest();
                    request.id = Guid.NewGuid().ToString();
                    request.method = "setsize";
                    request.param = new JObject
                    {
                        ["width"] = _wfh.PhysicalWidth,
                        ["height"] = _wfh.PhysicalHeight,
                        ["dpiRatio"] = _dpiRatioFirst,
                        ["curscaling"] = graphics.DpiScaleX,
                        ["prescaling"] = _previousDpi.DpiScaleX
                    };

                    ProcessDemoGuiBase.g_mapRequest[request.id] = request;
                    RpcNotifyAsync(request);
                });
            }
            catch (Exception ex)
            {
                _logger?.Log(CosmosLogLevel.Error, $"调整窗口大小失败 {ex.Message}");
            }
        }

        #region ICosmosAppWidget 必须实现的接口
        /// <summary>
        /// 组件第一次启动时会调用到该方法。可以在这个函数中做初始化操作。
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override async Task StartAsync(CancellationToken cancellationToken)
        {
            //绑定cosmos引擎提供的日志记录器
            _logger = ContextInjection.ThisAppContext.AppLogger;
            _logger?.Log(CosmosLogLevel.Information, "ComDemoGui 启动");
            await _baseImpl.StartAsync(cancellationToken);
        }

        /// <summary>
        /// 组件彻底关闭时会调用到该方法。可以在这个函数中做一些析构的操作。
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override async Task StopAsync(CancellationToken cancellationToken)
        {
            await _baseImpl.StopAsync(cancellationToken);
        }

        /// <summary>
        /// 该接口设计初衷是在组件最小化或者隐藏时调用，用于节省流量和cpu消耗，目前暂未实现
        /// 举例，行情买卖五档可在此函数中暂停订阅成交数据、停止发送UI交互事件
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override Task PauseAsync(CancellationToken cancellationToken)
        {
            return _baseImpl.PauseAsync(cancellationToken);
        }
        /// <summary>
        /// 该接口设计初衷是在组件重新显示时调用，目前暂未实现
        /// 举例，行情买卖五档可在此函数中重新订阅成交数据
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override Task ContinueAsync(CancellationToken cancellationToken)
        {
            return _baseImpl.ContinueAsync(cancellationToken);
        }

        /// <summary>
        /// 该接口定义了当前组件和实例上下文的注入点，这些上下文在构造后由Cosmos引擎自动注入
        /// 可在该成员中实现对上下文的初始化和修改
        /// </summary>
        public override ICosmosAppContextInjection ContextInjection
        {
            get
            {
                return _baseImpl.ContextInjection;
            }
            set
            {
                _baseImpl.ContextInjection = value;
            }
        }

        public override ICosmosAppAccessProvider AccessProvider 
        { 
            get => _baseImpl.AccessProvider; 
            set => _baseImpl.AccessProvider = value; 
        }
        #endregion

        #region WpfCosmosAppProcessWidget 必须实现的接口
        /// <summary>
        /// 接受其他组件调用方法
        /// </summary>
        public override Task<ICosmosRpcResponse> onInvoke(ICosmosRpcRequest parameter, ref bool bHandle)
        {
            return _baseImpl.onInvoke(parameter, ref bHandle);
        }

        /// <summary>
        /// 接受其他组件通知
        /// </summary>
        public override void onNotify(ICosmosRpcRequest parameter, ref bool bHandle)
        {
            _baseImpl.onNotify(parameter, ref bHandle);
        }

        /// <summary>
        /// 处理推送数据（进程间消息）
        /// </summary>
        protected override void HandlePush(ICosmosRpcPush data)
        {
            _baseImpl.HandlePush(data);
        }

        /// <summary>
        /// 处理通知（进程间消息）
        /// </summary>
        protected override void HandleNotify(ICosmosRpcRequest data)
        {
            _baseImpl.HandleNotify(data);
        }

        /// <summary>
        /// 处理调用（进程间消息）
        /// </summary>
        protected override async Task<ICosmosRpcResponse> HandleInvoke(ICosmosRpcRequest data)
        {
            return await _baseImpl.HandleInvoke(data);
        }

        /// <summary>
        /// 处理订阅（进程间消息）
        /// </summary>
        protected override void HandleSubscribe(ICosmosRpcRequest data)
        {
            _baseImpl.HandleSubscribe(data);
        }
        #endregion

        #region WPF 特定的实现

        public double _dpiRatioFirst = 1;
        private DpiScale _previousDpi;

        #endregion
    }
}
