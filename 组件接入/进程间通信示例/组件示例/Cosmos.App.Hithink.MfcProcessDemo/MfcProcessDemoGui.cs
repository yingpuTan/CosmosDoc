using System;
using System.Drawing;
using System.Windows;
using System.Windows.Shapes;
using Cosmos.App.Sdk.v1;
using Cosmos.App.Sdk.Windows;
using System.Reflection;
using System.IO;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using Path = System.IO.Path;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System.Text.Json.Nodes;
using System.Text.Json;
using System.Windows.Media;
using System.ComponentModel;
using Microsoft.Win32;
using System.Windows.Threading;


namespace Cosmos.App.Hithink.MfcProcessDemo
{
    public class MfcProcessDemoGui :
        WpfCosmosAppProcessWidget //进程间通讯基类，并且需要实现类中提供的抽象方法。
        
    {
        public MfcProcessDemoGui()
        {
            _wfh = new WpfCosmosHwndHost();
            _wfh.Margin = new Thickness(0, 0, 0, 0);
            /// 设置大小改变事件
            _wfh.SizeChanged += _wfh_SizeChanged;
            SystemParameters.StaticPropertyChanged += SystemParameters_StaticPropertyChanged;

            /// 设置窗口加载成功事件
            _wfh.Loaded += _wfh_Loaded;
            Content = _wfh;
        }

        WpfCosmosHwndHost _wfh { get; set; }
        private void _wfh_Loaded(object sender, RoutedEventArgs e)
        {
            _logger?.Log(CosmosLogLevel.Information, "Wfh_Loaded");

            //窗口创建成功、可以启动进程通信服务
            StartRpcServer();
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
                        ["curscaling"] = graphics.DpiScaleX
                    };

                    g_mapRequest[request.id] = request;
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
        }

        /// <summary>
        /// 组件彻底关闭时会调用到该方法。可以在这个函数中做一些析构的操作。
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override async Task StopAsync(CancellationToken cancellationToken)
        {
            //组件关闭、释放进程通信资源
            ReleaseRpcMnager();
        }

        /// <summary>
        /// 该接口设计初衷是在组件最小化或者隐藏时调用，用于节省流量和cpu消耗，目前暂未实现
        /// 举例，行情买卖五档可在此函数中暂停订阅成交数据、停止发送UI交互事件
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override Task PauseAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "ComDemoGui 暂停");
            return Task.CompletedTask;
        }
        /// <summary>
        /// 该接口设计初衷是在组件重新显示时调用，目前暂未实现
        /// 举例，行情买卖五档可在此函数中重新订阅成交数据
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override Task ContinueAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "ComDemoGui 继续运行");
            return Task.CompletedTask;
        }

        /// <summary>
        /// 该接口定义了当前组件和实例上下文的注入点，这些上下文在构造后由Cosmos引擎自动注入
        /// 可在该成员中实现对上下文的初始化和修改
        /// </summary>
        public override ICosmosAppContextInjection ContextInjection
        {
            get
            {
                return _ContextInjection;
            }
            set
            {
                _ContextInjection = value;
                _ContextInjection.ThisAppContext.GlobalContexts.VisualContext.ColorSchemeChanged += (s, v) =>
                {
                    ICosmosRpcRequest request = CreateRpcRequest();
                    request.method = "SchemeChanged";
                    request.param = new JObject()
                    {
                        ["theme"] = v as string
                    };
                    RpcNotifyAsync(request);
                };
            }
        }

        public override ICosmosAppAccessProvider AccessProvider { get; set; }
        #endregion

        #region WpfCosmosAppProcessWidget 必须实现的接口

        /// <summary>
        /// 接受其他组件调用方法
        /// </summary>
        public override Task<ICosmosRpcResponse> onInvoke(ICosmosRpcRequest parameter, ref bool bHandle)
        {
            //组件是否需要处理进程间通讯请求，如果要处理bHandle置为true，并且执行自己的业务代码，处理完成后返回resoinse、如果不处理返回null即可

            /*//处理进程间通讯请求,不往进程发送
            {
                bHandle = true;
                ICosmosRpcResponse response = _ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateResponseParameter();
                Console.WriteLine($"MfcProcessDemoGui 接收到其他组件发起请求  请求方法为：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}");
                response.code = 200;
                return Task.FromResult(response);
            }

            //处理进程间通讯请求,且进程发送
            {
                ICosmosRpcResponse response = _ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateResponseParameter();
                Console.WriteLine($"MfcProcessDemoGui 接收到其他组件发起请求  请求方法为：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}");
                response.code = 200;
                return Task.FromResult(response);
            }*/

            //不处理进程通讯进球，让进程自己处理
            {
                return null;
            }
        }

        /// <summary>
        /// 接受其他组件通知
        /// </summary>
        public override void onNotify(ICosmosRpcRequest parameter, ref bool bHandle)
        {
            //组件是否需要单独处理进程间通讯通知，如果要处理bHandle置为true，并且执行自己的业务代码、如果不处理返回null即可

           /* //处理进程间通讯请求,不往进程发送
            {
                bHandle = true;
                Console.WriteLine($"MfcProcessDemoGui 接收到其他组件发起通知 通知方法为：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}");
            }

            //处理进程间通讯请求,且进程发送
            {
                Console.WriteLine($"MfcProcessDemoGui 接收到其他组件发起通知 通知方法为：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}");
            }*/

            //不处理进程通讯进球，让进程自己处理
            {
                return;
            }
        }

        /// <summary>
        /// 处理推送数据（进程间消息）
        /// </summary>
        protected override void HandlePush(ICosmosRpcPush data)
        {
            string topic = data.topic;
            if (topic == "push_account")
            {
                // 处理推送账户信息
                string strID = (string)data.param["ID"];
                if (g_mapActInfo.ContainsKey(strID))
                {
                    g_mapActInfo[strID].Type = (int)data.param["Type"];
                    g_mapActInfo[strID].Status = (int)data.param["Status"];
                }
            }
        }

        /// <summary>
        /// 处理通知（进程间消息）
        /// </summary>
        protected override void HandleNotify(ICosmosRpcRequest data)
        {
            string method = data.method;
            if (method == "init_succ")
            {
                // 对方进程初始化完成，按需实现业务需求
                ChangeProcessWindowSize();
            }
            else if (method == "notf_test")
            {
                TimerCallback(data);
            }
            else if (method == "notf_sub")
            {
                // 对方初始化成功，可以向其发送通信，这里做如下业务操作
                // 1、订阅账户信息
                ICosmosRpcRequest sub = CreateRpcRequest();
                sub.id = Guid.NewGuid().ToString();
                sub.method = "sub_account";
                sub.param = new JObject
                {
                    ["ID"] = "123456" //订阅账户123456信息
                };

                g_mapRequest[sub.id] = sub;
                RpcSubscribeAsync(sub);


                // 2、查询账户信息（同步调用）
                ICosmosRpcRequest sync_param = CreateRpcRequest();
                sync_param.id = Guid.NewGuid().ToString();
                sync_param.method = "qry_account";
                sync_param.param = new JObject
                {
                    ["ID"] = "ALL" //查询所有账户信息
                };

                g_mapRequest[sync_param.id] = sync_param;
                ICosmosRpcResponse sync_ret = RpcInvokeAsync(sync_param).Result; //默认30秒超时
                {
                    // 处理返回结果 sync_ret
                    if (g_mapRequest.ContainsKey(sync_ret.id))
                    {
                        // 找到了请求上下文
                        if (sync_ret.code == 0)
                        {
                            // 返回业务成功                    
                            string async_method = g_mapRequest[sync_ret.id].method;
                            if (async_method == "qry_account")
                            {
                                JToken resultToken = sync_ret.result;

                                // 反序列化 result 属性
                                var deserializedAccounts = JsonConvert.DeserializeObject<List<AccountInfo>>(resultToken.ToString());
                                foreach (var account in deserializedAccounts)
                                {
                                    g_mapActInfo[account.ID] = account;
                                }
                            }
                        }
                        else
                        {
                            // 返回业务报错
                            // 按报错处理
                        }
                    }
                }

                // 3、查询账户信息（异步调用）
                QryAccount_InvokeAsync();
            }
        }

        /// <summary>
        /// 处理调用（进程间消息）
        /// </summary>
        protected override async Task<ICosmosRpcResponse> HandleInvoke(ICosmosRpcRequest data)
        {
            ICosmosRpcResponse resp = CreateRpcResponse();
            if (data.method == "requestTheme")
            {
                resp.code = 0;
                var theme = _ContextInjection.ThisAppContext.GlobalContexts.VisualContext.ColorScheme;
                resp.result = new JObject();
                resp.result["theme"] = theme;
            }
            else if (data.method.Contains("requestThemeRes"))
            {
                resp.code = 0;
                var dictionary = _ContextInjection.ThisAppContext.GlobalContexts.VisualContext.ThemeResources;
                resp.result = JToken.FromObject(dictionary);
            }
            else if (data.method == "test_invoke")
            {
                System.Threading.Thread.Sleep(5000);
                resp.code = 0;
                resp.result = new JObject();
                resp.result["theme"] = "onresp_test_invoke";
            }
            else if (data.method == "test_invoke_async")
            {
                System.Threading.Thread.Sleep(5000);
                resp.code = 0;
                resp.result = new JObject();
                resp.result["theme"] = "onresp_test_invoke_async";
            }

            return resp;
        }


        /// <summary>
        /// 处理订阅（进程间消息）
        /// </summary>
        protected override void HandleSubscribe(ICosmosRpcRequest data)
        {
            // 处理 客户端发送的 SUBS 事件（注：业务按需处理，以下为示例）
        }

        #endregion 
        #region 进程通信测试示例

        public double _dpiRatioFirst = 1;
        private DpiScale _previousDpi;
        async void StartRpcServer()
        {
            try
            {
                // 1.获取进程所在路径
                var clientDir = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content\dependents\TestMfc.exe");
                //clientDir = @"D:\git\itrader\Cosmos_yinhe\Source\RpcClient\x64\Debug\TestMfc.exe";
                // 2.手动获取当前WPF应用程序窗口句柄和真实窗口大小,组装命令行参数
                var hwnd = _wfh.Handle;
                var hwndGraphics = Graphics.FromHwnd(hwnd);
                _dpiRatioFirst = hwndGraphics.DpiX / 96;
                _previousDpi = VisualTreeHelper.GetDpi(this);
                var actualWidth = _wfh.PhysicalWidth * _dpiRatioFirst;
                var actualHeight = _wfh.PhysicalHeight * _dpiRatioFirst;
                string strCmd = $"{hwnd}|{(int)actualWidth}|{(int)actualHeight}";

                // 3、启动进程通讯服务，并将当前父窗口的进程id和窗口大小通知对方进程、返回对方进程id
                ProcessId = CreateRpcManager(clientDir, strCmd, _logger);
                if (ProcessId != 0)
                {
                    //创建成功，向cosmos引擎设置进程信息
                    ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.SetProcessInfo(ProcessId, this);
                }

                //4设置崩溃文件路径和前缀名
                dumpPath = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"dump");
                dumpName = "TestMfc";
            }
            catch(Exception ex)
            {
                _logger?.Log(CosmosLogLevel.Error, $"StartRpcServer:{ex.ToString()}");
            }

            _logger?.Log(CosmosLogLevel.Information, "StartRpcServer");
        }

        private async void ReleaseRpcMnager()
        {
            try
            {
                //组件关闭
                //1.向cosmos引擎取消注册的进程信息
                ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.RemoveProcessInfo(ProcessId);

                //2.发送关闭通知给进程
                ICosmosRpcRequest request = CreateRpcRequest();
                request.id = Guid.NewGuid().ToString();
                request.method = "close";

                g_mapRequest[request.id] = request;
                await RpcNotifyAsync(request);

            }
            catch (Exception ex)
            {
                _logger?.Log(CosmosLogLevel.Error, $"ReleaseRpcMnager:{ex.ToString()}");
            }

            _logger?.Log(CosmosLogLevel.Information, "ReleaseRpcMnager");
        }

        async Task<string> QryAccount_InvokeAsync()
        {
            // 异步查询账户信息
            ICosmosRpcRequest async_param = CreateRpcRequest();
            async_param.id = Guid.NewGuid().ToString();
            async_param.method = "qry_account";
            async_param.param = new JObject
            {
                ["ID"] = "123456",  //查询账户123456的信息
            };

            g_mapRequest[async_param.id] = async_param;
            ICosmosRpcResponse async_ret = await RpcInvokeAsync(async_param, 10000); //10秒超时
            {
                // 处理返回结果 async_ret
                if (g_mapRequest.ContainsKey(async_ret.id))
                {
                    // 找到了请求上下文
                    if (async_ret.code == 0)
                    {
                        // 返回业务成功                    
                        string async_method = g_mapRequest[async_ret.id].method;
                        if (async_method == "qry_account")
                        {
                            string ID = (string)async_ret.result["ID"];
                            if (g_mapActInfo.ContainsKey(ID))
                            {
                                g_mapActInfo[ID].Type = (int)async_ret.result["Type"];
                                g_mapActInfo[ID].Status = (int)async_ret.result["Status"];
                            }
                        }
                    }
                    else
                    {
                        // 返回业务报错
                        // 按报错处理
                    }
                }
            }

            return "OK";
        }
        private void TimerCallback(ICosmosRpcRequest data)
        {
            var request = ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateRequestParameter();
            request.method = "textchanged";
            request.id = ContextInjection.ThisInstanceContext.Id;
            request.param = data.param;
            InvokeType type = InvokeType.Global;
            {
                var result = ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.InvokeWidget(null, InvokeType.Global, request, false);
            }
            return;
        }

        #endregion

        #region 组件自定义属性

        /// 保存上下文信息
        /// </summary>
        private ICosmosAppContextInjection _ContextInjection;
        /// <summary>
        /// 日志记录
        /// </summary>
        private ICosmosAppLogger _logger { get; set; }

        //当前启动进程进程id
        private int ProcessId;
        public class AccountInfo
        {
            public string ID { get; set; }
            public int Type { get; set; } = 0;
            public int Status { get; set; } = 0;
        }

        static Dictionary<string, AccountInfo> g_mapActInfo = new Dictionary<string, AccountInfo>();
        static Dictionary<string, ICosmosRpcRequest> g_mapRequest = new Dictionary<string, ICosmosRpcRequest>();
        #endregion
    }
}
