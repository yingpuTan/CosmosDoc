using System;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using Cosmos.App.Sdk.v1;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Path = System.IO.Path;

namespace Cosmos.App.Hithink.ProcessDemo.Shared
{
    /// <summary>
    /// 进程通信Demo的共享辅助类，包含公用的业务逻辑代码
    /// </summary>
    public class ProcessDemoGuiBase
    {
        #region 组件自定义属性

        /// <summary>
        /// 保存上下文信息
        /// </summary>
        protected ICosmosAppContextInjection _ContextInjection;
        
        /// <summary>
        /// 日志记录
        /// </summary>
        protected ICosmosAppLogger _logger { get; set; }

        //当前启动进程进程id
        protected int ProcessId;
        
        public class AccountInfo
        {
            public string ID { get; set; }
            public int Type { get; set; } = 0;
            public int Status { get; set; } = 0;
        }

        public static Dictionary<string, AccountInfo> g_mapActInfo = new Dictionary<string, AccountInfo>();
        public static Dictionary<string, ICosmosRpcRequest> g_mapRequest = new Dictionary<string, ICosmosRpcRequest>();
        
        #endregion

        #region 接口 - 由外部提供框架特定的功能

        /// <summary>
        /// 获取窗口句柄的委托
        /// </summary>
        public Func<IntPtr> GetWindowHandle { get; set; }

        /// <summary>
        /// 获取物理宽度的委托
        /// </summary>
        public Func<double> GetPhysicalWidth { get; set; }

        /// <summary>
        /// 获取物理高度的委托
        /// </summary>
        public Func<double> GetPhysicalHeight { get; set; }

        /// <summary>
        /// 获取当前DPI缩放比例的委托
        /// </summary>
        public Func<double> GetCurrentDpiScale { get; set; }

        /// <summary>
        /// 获取之前的DPI缩放比例的委托
        /// </summary>
        public Func<double> GetPreviousDpiScale { get; set; }

        /// <summary>
        /// 设置之前的DPI缩放比例的委托
        /// </summary>
        public Action<double> SetPreviousDpiScale { get; set; }

        /// <summary>
        /// 设置首次DPI缩放比例的委托
        /// </summary>
        public Action<double> SetFirstDpiRatio { get; set; }

        /// <summary>
        /// 获取首次DPI缩放比例的委托
        /// </summary>
        public Func<double> GetFirstDpiRatio { get; set; }

        /// <summary>
        /// 在主线程上执行操作的委托
        /// </summary>
        public Action<Action> InvokeOnMainThread { get; set; }

        /// <summary>
        /// 创建RPC请求的委托
        /// </summary>
        public Func<ICosmosRpcRequest> CreateRpcRequest { get; set; }

        /// <summary>
        /// 创建RPC响应的委托
        /// </summary>
        public Func<ICosmosRpcResponse> CreateRpcResponse { get; set; }

        /// <summary>
        /// 创建RPC管理器的委托
        /// </summary>
        public Func<string, string, ICosmosAppLogger, int> CreateRpcManager { get; set; }

        /// <summary>
        /// RPC通知异步的委托
        /// </summary>
        public Func<ICosmosRpcRequest, Task> RpcNotifyAsync { get; set; }

        /// <summary>
        /// RPC调用异步的委托
        /// </summary>
        public Func<ICosmosRpcRequest, int, Task<ICosmosRpcResponse>> RpcInvokeAsync { get; set; }

        /// <summary>
        /// RPC订阅异步的委托
        /// </summary>
        public Func<ICosmosRpcRequest, Task> RpcSubscribeAsync { get; set; }

        /// <summary>
        /// 改变进程窗口大小的委托
        /// </summary>
        public Action ChangeProcessWindowSize { get; set; }

        #endregion

        #region ICosmosAppWidget 必须实现的接口

        /// <summary>
        /// 组件第一次启动时会调用到该方法。可以在这个函数中做初始化操作。
        /// </summary>
        public virtual async Task StartAsync(CancellationToken cancellationToken)
        {
            //绑定cosmos引擎提供的日志记录器
            _logger = ContextInjection.ThisAppContext.AppLogger;
            _logger?.Log(CosmosLogLevel.Information, "ProcessDemoGui 启动");
        }

        /// <summary>
        /// 组件彻底关闭时会调用到该方法。可以在这个函数中做一些析构的操作。
        /// </summary>
        public virtual async Task StopAsync(CancellationToken cancellationToken)
        {
            //组件关闭、释放进程通信资源
            ReleaseRpcMnager();
        }

        /// <summary>
        /// 该接口设计初衷是在组件最小化或者隐藏时调用，用于节省流量和cpu消耗，目前暂未实现
        /// 举例，行情买卖五档可在此函数中暂停订阅成交数据、停止发送UI交互事件
        /// </summary>
        public virtual Task PauseAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "ProcessDemoGui 暂停");
            return Task.CompletedTask;
        }

        /// <summary>
        /// 该接口设计初衷是在组件重新显示时调用，目前暂未实现
        /// 举例，行情买卖五档可在此函数中重新订阅成交数据
        /// </summary>
        public virtual Task ContinueAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "ProcessDemoGui 继续运行");
            return Task.CompletedTask;
        }

        /// <summary>
        /// 该接口定义了当前组件和实例上下文的注入点，这些上下文在构造后由Cosmos引擎自动注入
        /// 可在该成员中实现对上下文的初始化和修改
        /// </summary>
        public virtual ICosmosAppContextInjection ContextInjection
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

        public virtual ICosmosAppAccessProvider AccessProvider { get; set; }

        #endregion

        #region WpfCosmosAppProcessWidget 必须实现的接口

        /// <summary>
        /// 接受其他组件调用方法
        /// </summary>
        public virtual Task<ICosmosRpcResponse> onInvoke(ICosmosRpcRequest parameter, ref bool bHandle)
        {
            //不处理进程通讯请求，让进程自己处理
            return null;
        }

        /// <summary>
        /// 接受其他组件通知
        /// </summary>
        public virtual void onNotify(ICosmosRpcRequest parameter, ref bool bHandle)
        {
            //不处理进程通讯通知，让进程自己处理
            return;
        }

        /// <summary>
        /// 处理推送数据（进程间消息）
        /// </summary>
        public virtual void HandlePush(ICosmosRpcPush data)
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
        public virtual void HandleNotify(ICosmosRpcRequest data)
        {
            string method = data.method;
            if (method == "init_succ")
            {
                // 对方进程初始化完成，按需实现业务需求
                ChangeProcessWindowSize?.Invoke();
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
                ICosmosRpcResponse sync_ret = RpcInvokeAsync(sync_param, 30000).Result; //默认30秒超时
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
                _ = QryAccount_InvokeAsync();
            }
        }

        /// <summary>
        /// 处理调用（进程间消息）
        /// </summary>
        public virtual async Task<ICosmosRpcResponse> HandleInvoke(ICosmosRpcRequest data)
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
        public virtual void HandleSubscribe(ICosmosRpcRequest data)
        {
            // 处理 客户端发送的 SUBS 事件（注：业务按需处理，以下为示例）
        }

        #endregion

        #region 进程通信测试示例

        /// <summary>
        /// 启动RPC服务器
        /// </summary>
        public virtual async void StartRpcServer()
        {
            try
            {
                // 1.获取进程所在路径
                var clientDir = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content\dependents\TestWpf.exe");
                //var clientDir = @"D:\git\itrader\Cosmos_yinhe\Source\RpcClient\TestWpf\bin\Debug\net7.0-windows\win-x64\TestWpf.exe";
                // 2.手动获取当前应用程序窗口句柄和真实窗口大小,组装命令行参数
                var hwnd = GetWindowHandle();
                var currentDpi = GetCurrentDpiScale();
                SetPreviousDpiScale(currentDpi);
                SetFirstDpiRatio(currentDpi);
                var actualWidth = GetPhysicalWidth() * currentDpi;
                var actualHeight = GetPhysicalHeight() * currentDpi;
                string strCmd = $"{hwnd}|{(int)actualWidth}|{(int)actualHeight}";

                // 3、启动进程通讯服务，并将当前父窗口的进程id和窗口大小通知对方进程、返回对方进程id
                ProcessId = CreateRpcManager(clientDir, strCmd, _logger);
                if (ProcessId != 0)
                {
                    //创建成功，向cosmos引擎设置进程信息
                    ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.SetProcessInfo(ProcessId, _widgetInstance);
                }

                //4、 设置崩溃文件路径和前缀名
                var dumpPath = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"dump");
                var dumpName = "TestWpf";
            }
            catch (Exception ex)
            {
                _logger?.Log(CosmosLogLevel.Error, $"StartRpcServer:{ex.ToString()}");
            }

            _logger?.Log(CosmosLogLevel.Information, "StartRpcServer");
        }

        /// <summary>
        /// Widget实例，用于传递给引擎
        /// </summary>
        public object _widgetInstance { get; set; }

        /// <summary>
        /// 释放RPC管理器
        /// </summary>
        public virtual async void ReleaseRpcMnager()
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

        /// <summary>
        /// 异步查询账户信息
        /// </summary>
        public virtual async Task<string> QryAccount_InvokeAsync()
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

        #endregion
    }
}

