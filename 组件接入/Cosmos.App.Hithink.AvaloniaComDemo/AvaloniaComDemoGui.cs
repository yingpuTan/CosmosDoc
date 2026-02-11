using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;
using System.Text.Json;
using System.Text.Json.Nodes;
using Newtonsoft.Json.Linq;

using Cosmos.App.Sdk.v1;
using Cosmos.App.Sdk.Windows;
using Cosmos.DataAccess.v1;
using Cosmos.DataAccess.v1.CosmosIntegration;
using Cosmos.App.Sdk.v1.Primitives;
using Cosmos.App.Sdk.v1.Controls.WebView;
using Cosmos.DataAccess.v1.Protocol;
using Cosmos.DataAccess.Trade.v1.CosmosIntegration;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Protocol;
using Cosmos.App.Hithink.Demo.Shared;
using Avalonia.Controls;

namespace Cosmos.App.Hithink.AvaloniaComDemo
{
    public class AvaloniaComDemoGui :
        AvaloniaCosmosAppWidget, //组件基类，必须继承自此类，并且需要实现类中提供的抽象方法。
        ICosmosDataInteraction, //需要访问行情数据底座，需实现此接口
        ICosmosAppAccessor, //组件业务接口方法，提供方法给外部调用们需要实现子接口
        ICosmosTradeDataInteraction,
        ICosmosWidgetComunication
    {
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
            _logger?.Log(CosmosLogLevel.Information, "AvaloniaComDemoGui 启动");

            var url = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content\resources\index.html");

            //创建浏览器实例
            _webView = ContextInjection.ThisAppContext.GlobalContexts.EngineContext.WebViewFactory.CreateWebView(url);

            //绑定接收页面消息的事件
            _webView.WebMessageReceived += _webView_WebMessageReceived;
            var config = ReadFromResource("Config.json");

            //处理页面初始化完成事件，与页面进行数据交互需要在这个时间通知完成后进行
            _webView.InitializationCompleted += _webView_InitializationCompleted;

            /// 绑定行情数据访问器
            _dataAccessor = AccessorsInjection?.DataAccessor;
            _tradeDataAccessor = TradeAccessorsInjection?.DataAccessor;
            _comDemo = new ComDemo(_dataAccessor, _logger, _ContextInjection, _tradeDataAccessor);

            if (_comDemo is null)
                throw new NullReferenceException($"{nameof(_comDemo)} null on AvaloniaComDemoGui");

            //将web页面放到到需要显示的位置
            //当整个页面是web页面时，可以直接赋值给Content，本例子只显示在某个特定的区域
            _comDemo.SetWebContent(_webView);
            Content = _comDemo;

            dumpName = "Com";
            dumpPath = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"dump");
        }

        /// <summary>
        /// 组件彻底关闭时会调用到该方法。可以在这个函数中做一些析构的操作。
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public async override Task StopAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "AvaloniaComDemoGui 退出");
            if (_tradeDataSubscription != null)
            {
                //_tradeDataSubscription.UnsubscribeAsync();
            }
            await _comDemo.stop();
            _logger?.Log(CosmosLogLevel.Information, "AvaloniaComDemoGui 退出完成");
        }

        /// <summary>
        /// 该接口设计初衷是在组件最小化或者隐藏时调用，用于节省流量和cpu消耗，目前暂未实现
        /// 举例，行情买卖五档可在此函数中暂停订阅成交数据、停止发送UI交互事件
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public override Task PauseAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "AvaloniaComDemoGui 暂停");
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
            _logger?.Log(CosmosLogLevel.Information, "AvaloniaComDemoGui 继续运行");
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
                OnContextInjection(value);

                ///创建组件访问器，并绑定到该组件上
                AccessProvider = new ComDemoProvider()
                {
                    AppAccessor = this
                };
            }
        }

        public override ICosmosAppAccessProvider AccessProvider { get; set; }
        #endregion

        #region ICosmosWidgetComunication 必须实现的接口
        public Task<ICosmosRpcResponse> OnInvoke(ICosmosRpcRequest parameter)
        {
            ICosmosRpcResponse response = ContextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateResponseParameter();

            var txt = $"AvaloniaComDemoGui 接收到其他组件发起请求  请求方法为：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}";
            _comDemo.ReceiveTxt(txt);
            Console.WriteLine(txt);
            response.code = 200;
            return Task.FromResult(response);
        }

        public void OnNotify(ICosmosRpcRequest parameter)
        {
            var txt = $"AvaloniaComDemoGui 接收到其他组件发起通知 通知方法为 ：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}";
            _comDemo.ReceiveTxt(txt);
            Console.WriteLine(txt);
        }
        #endregion

        #region ICosmosAppAccessor 必须实现的接口
        /// <summary>
        /// 提供给宿主invoke调用的方法（同步方法）
        /// </summary>
        /// <param name="cosmosOperator">调用者</param>
        /// <param name="method">方法名</param>
        /// <param name="parameters">参数、json串，需要组件开发者自己定义好协议，提供给外部</param>
        /// <param name="timeoutSpan"></param>
        /// <returns></returns>
        /// <exception cref="NotImplementedException"></exception>
        public async Task<string> InvokeAsync(string cosmosOperator, string method, string parameters, int timeoutSpan = 0)
        {
            string strResult = string.Empty;
            switch (method.ToLower())
            {
                //获取text框文本
                case "getname":
                    {
                        strResult = "AvaloniaComDemo";
                        break;
                    }

                default:
                    throw new NotImplementedException();
            }

            return strResult;
        }

        /// <summary>
        /// cosmos引擎暂不支持，可暂不实现
        /// </summary>
        /// <param name="cosmosOperator"></param>
        /// <param name="method"></param>
        /// <param name="parameters"></param>
        /// <param name="returnValueHandler"></param>
        /// <param name="timeoutSpan"></param>
        /// <returns></returns>
        /// <exception cref="NotImplementedException"></exception>
        public Task InvokeReturnAsync(string cosmosOperator, string method, string parameters, Action<string> returnValueHandler, int timeoutSpan = 0)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 提供给宿主notify调用的方法（异步方法）
        /// </summary>
        /// <param name="cosmosOperator">调用者</param>
        /// <param name="title">通知方法名</param>
        /// <param name="notification">通知内容</param>
        /// <param name="timeoutSpan"></param>
        /// <returns></returns>
        /// <exception cref="NotImplementedException"></exception>
        public Task NotifyAsync(string cosmosOperator, string title, string notification, int timeoutSpan = 0)
        {
            switch (title.ToLower())
            {
                //设置text框文本
                case "setname":
                    var setTextRequest = JsonSerializer.Deserialize<SetTextRequest>(notification);
                    Console.WriteLine($"SetName:{setTextRequest?.Name}");
                    break;
                default:
                    throw new NotImplementedException();
            }

            return Task.CompletedTask;
        }

        /// <summary>
        /// 提供宿主订阅调用的方法（异步方法）
        /// </summary>
        /// <param name="cosmosOperator">订阅者</param>
        /// <param name="subscriptionTopic">订阅主题</param>
        /// <param name="parameters">订阅的参数</param>
        /// <param name="pushDataHandler">推送回调</param>
        /// <param name="timeoutSpan"></param>
        /// <returns>订阅成功id唯一标识，后续推送根据改id进行推送</returns>
        /// <exception cref="NotImplementedException"></exception>
        public async Task<string> SubscribeAsync(string cosmosOperator, string subscriptionTopic, string parameters, ISubscriberRaw<string, string, string, string, string, int>.PushDataHandler pushDataHandler, int timeoutSpan = 0)
        {
            string strUUid = string.Empty;
            switch (subscriptionTopic.ToLower())
            {
                //设置text框文本
                case "textchange":
                    strUUid = _comDemo.SubscribeTextChange(cosmosOperator, pushDataHandler);
                    break;
                default:
                    throw new NotImplementedException();
            }

            return strUUid;
        }
        /// <summary>
        /// 取消订阅
        /// </summary>
        /// <param name="cosmosOperator">调用者</param>
        /// <param name="subscriptionId">订阅id</param>
        /// <param name="timeoutSpan"></param>
        /// <returns></returns>
        public Task UnsubscribeAsync(string cosmosOperator, string subscriptionId, int timeoutSpan = 0)
        {
            _comDemo.UnSubscribeTextChange(subscriptionId);
            return Task.CompletedTask;
        }
        #endregion

        #region js调用c#方法

        private async void _webView_WebMessageReceived(object? sender, WebViewWebMessageReceivedEventArgs args)
        {
            try
            {
                //解析请求参数
                ContextInjection.ThisAppContext.AppLogger?.Log(CosmosLogLevel.Information, $"WebMessageReceived args:{args?.WebMessageAsJson}");
                var message = JsonSerializer.Deserialize<List<object>>(args.WebMessageAsJson);
                var requestJson = message[0];

                if (string.IsNullOrEmpty(requestJson?.ToString()))
                {
                    return;
                }
                //处理请求
                var result = await MessageProcessHandler(requestJson.ToString());
                //请求成功应答
                if (result.Item1 == 200)
                {
                    await _webView.ExecuteScript(args._SussFunc, result.Item2);
                }
                //请求错误应答
                else
                {
                    await _webView.ExecuteScript(args._FailFunc, result.Item1, result.Item2);
                }
            }
            catch (Exception ex)
            {
                ContextInjection.ThisAppContext.AppLogger?.Log(CosmosLogLevel.Error, $"WebMessageReceived Error:{ex}");
            }
        }

        private async Task<(int, string)> MessageProcessHandler(string requestJson)
        {
            ContextInjection.ThisAppContext.AppLogger?.Log(CosmosLogLevel.Information, $"MessageProcessHandler Request:{requestJson}");
            var errCode = 200;
            var msg = string.Empty;
            object result = string.Empty;
            try
            {
                // 解析请求参数
                var request = JsonSerializer.Deserialize<JsRequest>(requestJson);

                //请求类型为空，返回错误
                if (string.IsNullOrEmpty(request?.Type))
                {
                    return (500, "type is empty");
                }

                switch (request.Type.ToLower())
                {
                    case "getthemeresources":
                        result = ContextInjection.ThisAppContext.GlobalContexts.VisualContext.ThemeResources;
                        break;
                    default:
                        return (500, "type is not valid");
                }
                if (result != default)
                {
                    msg = Newtonsoft.Json.JsonConvert.SerializeObject(result);
                }
            }
            catch (Exception ex)
            {
                errCode = 500;
                msg = $"Type:{ex.GetType()}; Message:{ex.Message}";
                ContextInjection.ThisAppContext.AppLogger?.Log(CosmosLogLevel.Error, $"MessageProcessHandler Error:{ex}");
            }

            ContextInjection.ThisAppContext.AppLogger?.Log(CosmosLogLevel.Information, $"MessageProcessHandler Record: {requestJson}; \n Response: {msg}");
            return (errCode, msg);
        }

        #endregion

        #region c#调用js方法

        //浏览器初始化完成
        private async void _webView_InitializationCompleted(object? sender, WebViewInitializationCompletedEventArgs e)
        {
            if (e.IsSuccess)
            {
                //调用js的GetAdd方法
                var result = await _webView.ExecuteScriptAsync($"GetAdd(10,20)");
                Console.WriteLine($"Call JS Function Add, Result{result}");
            }
        }

        #endregion

        #region 组件自定义属性
        /// <summary>
        /// 组件界面对象
        /// </summary>
        private ComDemo _comDemo;

        /// <summary>
        /// 保存上下文信息
        /// </summary>
        private ICosmosAppContextInjection _ContextInjection;

        /// <summary>
        /// 日志记录
        /// </summary>
        private ICosmosAppLogger _logger { get; set; }

        /// <summary>
        /// Cosmos引擎行情数据访问器
        /// </summary>
        private IDataAccessor _dataAccessor { get; set; }

        /// <summary>
        /// Cosmos引擎交易数据访问器
        /// </summary>
        private ITradeDataAccessor _tradeDataAccessor { get; set; }

        /// <summary>
        /// Cosmos引擎访问器
        /// </summary>
        public ICosmosAccessorsInjection AccessorsInjection { get; set; }

        public ICosmosTradeAccessorsInjection TradeAccessorsInjection { get; set; }

        /// <summary>
        /// 浏览器对象，组件为cef页面时需要用到该对象
        /// </summary>
        public IWebView _webView;

        private ITradeDataSubscription _tradeDataSubscription;
        #endregion

        #region 组件自定义方法
        private void OnContextInjection(ICosmosAppContextInjection injection)
        {
            /// 绑定主题变化事件, 当宿主通知需要切换主题时触发
            injection.ThisAppContext.GlobalContexts.VisualContext.ColorSchemeChanged += ColorSchemeChanged;
        }

        /// <summary>
        /// 主题变化事件,可在该函数中调整组件样式
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ColorSchemeChanged(object? sender, string e)
        {
            //处理主题变化
        }

        // 从资源中读取配置文件
        public Config ReadFromResource(string fileName)
        {
            var assembly = Assembly.GetExecutingAssembly();
            var configNames = assembly.GetManifestResourceNames();
            var configName = configNames.First(t => t.Contains(fileName));
            var value = assembly.GetManifestResourceStream(configName);
            var config = JsonSerializer.Deserialize<Config>(value);
            return config;
        }
        public Task<bool> PreClose()
        {
            return Task.FromResult(result: true);
        }

        #endregion
    }
}

