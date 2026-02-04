using System;
using System.IO;
using System.Reflection;
using Cosmos.App.Sdk.v1;
using Cosmos.App.Sdk.Windows;
using Newtonsoft.Json.Linq;
using Cosmos.App.Hithink.ProcessDemo.Shared;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Platform;
using Path = System.IO.Path;

namespace Cosmos.App.Hithink.QtProcessDemo
{
    public class QtProcessDemoGui :
        WpfCosmosAppProcessWidget //进程间通讯基类，支持跨平台
    {
        private ProcessDemoGuiBase _baseImpl;
        private NativeControlHost _nativeHost;
        private double _dpiRatioFirst = 1;
        private double _previousDpiScale = 1.0;

        public QtProcessDemoGui()
        {
            _nativeHost = new NativeControlHost();
            _nativeHost.Margin = new Thickness(0);
            _nativeHost.SizeChanged += NativeHost_SizeChanged;
            _nativeHost.AttachedToVisualTree += NativeHost_AttachedToVisualTree;
            Content = _nativeHost;

            _baseImpl = new ProcessDemoGuiBase();
            InitializeBaseImpl();
        }

        private void InitializeBaseImpl()
        {
            _baseImpl._widgetInstance = this;
            _baseImpl.GetWindowHandle = () => GetWindowHandle();
            _baseImpl.GetPhysicalWidth = () => _nativeHost.Bounds.Width;
            _baseImpl.GetPhysicalHeight = () => _nativeHost.Bounds.Height;
            _baseImpl.GetCurrentDpiScale = () => GetCurrentDpiScale();
            _baseImpl.GetPreviousDpiScale = () => _previousDpiScale;
            _baseImpl.SetPreviousDpiScale = (scale) => _previousDpiScale = scale;
            _baseImpl.SetFirstDpiRatio = (ratio) => _dpiRatioFirst = ratio;
            _baseImpl.GetFirstDpiRatio = () => _dpiRatioFirst;
            _baseImpl.InvokeOnMainThread = (action) => Avalonia.Threading.Dispatcher.UIThread.Post(action);
            _baseImpl.CreateRpcRequest = () => CreateRpcRequest();
            _baseImpl.CreateRpcResponse = () => CreateRpcResponse();
            _baseImpl.CreateRpcManager = (clientDir, strCmd, logger) => CreateRpcManager(clientDir, strCmd, logger);
            _baseImpl.RpcNotifyAsync = (request) => RpcNotifyAsync(request);
            _baseImpl.RpcInvokeAsync = (request, timeout) => RpcInvokeAsync(request, timeout);
            _baseImpl.RpcSubscribeAsync = (request) => RpcSubscribeAsync(request);
            _baseImpl.ChangeProcessWindowSize = () => ChangeProcessWindowSize();
        }

        private IntPtr GetWindowHandle()
        {
            if (this.GetVisualRoot() is Window window)
            {
                var platformHandle = window.TryGetPlatformHandle();
                if (platformHandle != null)
                {
                    return platformHandle.Handle;
                }
            }
            return IntPtr.Zero;
        }

        private double GetCurrentDpiScale()
        {
            if (this.GetVisualRoot() is Window window)
            {
                return window.RenderScaling;
            }
            return 1.0;
        }

        private void NativeHost_AttachedToVisualTree(object? sender, Avalonia.VisualTreeAttachmentEventArgs e)
        {
            _logger?.Log(CosmosLogLevel.Information, "QtProcessDemoGui NativeHost_AttachedToVisualTree");

            var clientDir = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content/dependents/TestQt");
            //窗口创建成功、可以启动进程通信服务
            _baseImpl.StartRpcServer(clientDir);
        }

        private void NativeHost_SizeChanged(object? sender, SizeChangedEventArgs e)
        {
            ChangeProcessWindowSize();
        }

        private void ChangeProcessWindowSize()
        {
            try
            {
                Avalonia.Threading.Dispatcher.UIThread.Post(() =>
                {
                    var currentDpi = GetCurrentDpiScale();
                    ICosmosRpcRequest request = CreateRpcRequest();
                    request.id = Guid.NewGuid().ToString();
                    request.method = "setsize";
                    request.param = new JObject
                    {
                        ["width"] = _nativeHost.Bounds.Width,
                        ["height"] = _nativeHost.Bounds.Height,
                        ["dpiRatio"] = _dpiRatioFirst,
                        ["curscaling"] = currentDpi,
                        ["prescaling"] = _previousDpiScale
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
        public override async Task StartAsync(CancellationToken cancellationToken)
        {
            await _baseImpl.StartAsync(cancellationToken);
        }

        /// <summary>
        /// 组件彻底关闭时会调用到该方法。可以在这个函数中做一些析构的操作。
        /// </summary>
        public override async Task StopAsync(CancellationToken cancellationToken)
        {
            await _baseImpl.StopAsync(cancellationToken);
        }

        /// <summary>
        /// 该接口设计初衷是在组件最小化或者隐藏时调用，用于节省流量和cpu消耗，目前暂未实现
        /// </summary>
        public override Task PauseAsync(CancellationToken cancellationToken)
        {
            return _baseImpl.PauseAsync(cancellationToken);
        }

        /// <summary>
        /// 该接口设计初衷是在组件重新显示时调用，目前暂未实现
        /// </summary>
        public override Task ContinueAsync(CancellationToken cancellationToken)
        {
            return _baseImpl.ContinueAsync(cancellationToken);
        }

        /// <summary>
        /// 该接口定义了当前组件和实例上下文的注入点，这些上下文在构造后由Cosmos引擎自动注入
        /// </summary>
        public override ICosmosAppContextInjection ContextInjection
        {
            get => _baseImpl.ContextInjection;
            set => _baseImpl.ContextInjection = value;
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
    }
}
