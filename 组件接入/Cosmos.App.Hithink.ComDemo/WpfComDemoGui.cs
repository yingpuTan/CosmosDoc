using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using Cosmos.App.Sdk.v1;
using Cosmos.App.Sdk.Windows;
using System.Windows.Controls;
using Cosmos.DataAccess.v1;
using Cosmos.DataAccess.v1.CosmosIntegration;
using System.Text.Json;
using Cosmos.App.Sdk.v1.Primitives;
using Cosmos.App.Sdk.v1.Controls.WebView;
using System.Reflection;
using Cosmos.DataAccess.v1.Protocol;
using Cosmos.DataAccess.Trade.v1.CosmosIntegration;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System.Text.Json.Nodes;
using Newtonsoft.Json.Linq;
using System.Text.Encodings.Web;
using System.Text.Unicode;

namespace Cosmos.App.Hithink.ComDemo
{
    /// <summary>
    /// 配置
    /// </summary>
    public class Config
    {
        /// <summary>
        /// 页面地址
        /// </summary>
        public String Url { get; set; }

        /// <summary>
        /// 域名
        /// </summary>
        public String Domain { get; set; }
    }

    internal class JsRequest
    {
        public string Type { get; set; }

        public string Param { get; set; }
    }

    internal class GetUserIdResponse
    {
        public string UserID { get; set; }
    }

    internal class AccountInfo
    {
        public string AccountName { get; set; }
        public string Account { get; set; }
        public string Qsid { get; set; }
        public string State { get; set; }
    }

    public class WpfComDemoGui : 
        WpfCosmosAppWidget, //组件基类，必须继承自此类，并且需要实现类中提供的抽象方法。
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
            _logger?.Log(CosmosLogLevel.Information,  "ComDemoGui 启动");

            var url = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content\resources\index.html");

            //创建浏览器实例
            _webView = ContextInjection.ThisAppContext.GlobalContexts.EngineContext.WebViewFactory.CreateWebView(url);

            //绑定接收页面消息的事件
            _webView.WebMessageReceived += _webView_WebMessageReceived;
            var config = ReadFromResource("Config.json");

            //处理页面初始化完成事件，与页面进行数据交互需要在这个时间通知完成后进行
            _webView.InitializationCompleted += _webView_InitializationCompleted;

            /// 绑定行情数据访问器
            _dataAccessor =  AccessorsInjection?.DataAccessor;
            _tradeDataAccessor = TradeAccessorsInjection?.DataAccessor;
            _comDemo = new ComDemo(_dataAccessor, _logger, _ContextInjection, _tradeDataAccessor);

            if (_comDemo is null)
                throw new NullReferenceException($"{nameof(_comDemo)} null on ComDemoGui");

            //将web页面放到到需要显示的位置
            //当整个页面是web页面时，可以直接赋值给Content，本例子只显示在某个特定的区域
            _comDemo.SetWebContent(_webView);
            Content = _comDemo;

            dumpName = "Com";
            dumpPath = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"dump");
            //测试与宿主通信
            //await TestSendHost();

            //测试行情数据源接口
            // TestHqDataInerface();
        }

        /// <summary>
        /// 组件彻底关闭时会调用到该方法。可以在这个函数中做一些析构的操作。
        /// </summary>
        /// <param name="cancellationToken"></param>
        /// <returns></returns>
        public async override Task StopAsync(CancellationToken cancellationToken)
        {
            _logger?.Log(CosmosLogLevel.Information, "ComDemoGui 退出");
            if(_tradeDataSubscription != null)
            {
                //_tradeDataSubscription.UnsubscribeAsync();
            }
            await _comDemo.stop();
            _logger?.Log(CosmosLogLevel.Information, "ComDemoGui 退出完成");
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
           
            var txt = $"WpfComDemoGui 接收到其他组件发起请求  请求方法为：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}";
            _comDemo.ReceiveTxt(txt);
            Console.WriteLine(txt);
            response.code = 200;
            return Task.FromResult(response);
        }

        public void OnNotify(ICosmosRpcRequest parameter)
        {
            var txt = $"WpfComDemoGui 接收到其他组件发起通知 通知方法为 ：{parameter.method} ，参数为 {parameter.param.ToString()}, 来源{parameter.id}";
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
                        strResult = "ComDemo";
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

        internal class SetTextRequest
        {
            public string Name { get; set; }
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
                    Console.WriteLine($"SetName:{setTextRequest.Name}");
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
                if (string.IsNullOrEmpty(request.Type))
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
            if(e.IsSuccess)
            {
                //第二中加载html页面方法
                {
                    // 获取url地址
                    //var url = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location))), @"content\resources\index.html");

                    //从资源中获取需要打开的页面地址、也可以直接写死一个地址
                    //_webView.Navigate(url);
                }
                

                //调用js的GetAdd方法
                var result = await _webView.ExecuteScriptAsync($"GetAdd(10,20)");
                Console.WriteLine($"Call JS Function Add, Result{result}");
            }
        }

        #endregion

        #region 向宿主发送请求，订阅等数据
        public async Task TestSendHost()
        {
            //向宿主发送请求
            {
                //获取当前组件实例id
                var cosmos = Convert.ToString(_ContextInjection.ThisInstanceContext.InstanceId);
                //向宿主获取userid信息、请求应答格式查看宿主demo应答格式
                var userid = await _ContextInjection.ThisAppContext.GlobalContexts.ProductContext.ProductAccessor.InvokeAsync(cosmos, "getUserID", "", 0);
                if (userid != null)
                {
                    // 解析请求参数
                    var response = JsonSerializer.Deserialize<GetUserIdResponse>(userid);
                    if (response != null)
                    {
                        Console.WriteLine($"userid:{response.UserID}");
                    }
                }
                //向宿主获取账户信息、请求应答格式查看宿主demo应答格式
                var accounts = await _ContextInjection.ThisAppContext.GlobalContexts.ProductContext.ProductAccessor.InvokeAsync(cosmos, "getAccounts", "", 0);
                if (userid != null)
                {
                    // 解析请求参数
                    var response = JsonSerializer.Deserialize<List<AccountInfo>>(accounts);
                    if (response != null)
                    {
                        foreach (var account in response)
                        {
                            Console.WriteLine($"AccountName:{account.AccountName}, Account:{account.Account}, Account:{account.Qsid}, Account:{account.State} ");
                        }
                    }
                }
            }

            //向宿主发起通知
            {
                //获取当前组件实例id
                var cosmos = Convert.ToString(_ContextInjection.ThisInstanceContext.InstanceId);
                var jsNotify = new JsonObject();
                jsNotify["name"] = "comDemo";
                string strNotify = JsonSerializer.Serialize(jsNotify);
                //向宿主发送设置名字消息、请求应答格式查看宿主demo应答格式
                _ContextInjection.ThisAppContext.GlobalContexts.ProductContext.ProductAccessor.NotifyAsync(cosmos, "SetName", strNotify, 0);
            }

            //向宿主发起订阅
            {
                //获取当前组件实例id
                var cosmos = Convert.ToString(_ContextInjection.ThisInstanceContext.InstanceId);
                //向宿主发送设置名字消息、请求应答格式查看宿主demo应答格式
                _ContextInjection.ThisAppContext.GlobalContexts.ProductContext.ProductAccessor.SubscribeAsync(cosmos, "SubUserID", "", (string subscriberId, string subscriptionId, string pushData)=>
                {
                    //打印推送消息
                    Console.WriteLine($"subscriberId:{subscriberId}, subscriptionId{subscriptionId},pushData{pushData}");

                    //取消订阅
                    _ContextInjection.ThisAppContext.GlobalContexts.ProductContext.ProductAccessor.UnsubscribeAsync(subscriberId, subscriptionId);
                });
            }
        }
        #endregion

        #region 行情数据源接口测试示例
        private async Task TestHqDataInerface()
        {
            ///根据输入文本模糊匹配股票数据
            {
                var fuzzySearchParameters = _dataAccessor.DataProvider.CreateFuzzySearchParameters();
                fuzzySearchParameters.Keyword = "0";
                var fuzzyResult = await _dataAccessor.DataProvider.SearchFuzzyAsync(fuzzySearchParameters);
                foreach(var assetInfo in fuzzyResult.MatchedAssetInfos)
                {
                    Console.WriteLine($"代码：{assetInfo.AssetId.SymbolValue}, 市场：{assetInfo.AssetId.MarketId}");
                }
            }

            ///查询行情逐笔成交数据
            {
                var fieldIds = _dataAccessor.FieldIds;
                var tradeHistoryParameters = _dataAccessor.DataProvider.CreateTradeHistoryParameters();
                tradeHistoryParameters.RequestCount = 50;
                tradeHistoryParameters.AssetId = _dataAccessor.DataProvider.ToAssetId("USHA600000");
                tradeHistoryParameters.FieldIds = new[]
                {
                    fieldIds.DateTime, fieldIds.Volclass, fieldIds.LastPrice, fieldIds.LatestTransactionVolume, fieldIds.TransactionCount
                };

                //获取需要查询字段的详细信息
                {
                    foreach(var fieldId in tradeHistoryParameters.FieldIds)
                    {
                        var filedInfo = _dataAccessor.DataProvider.QueryFieldInfo(fieldId);
                        Console.WriteLine($"字段中文名：{filedInfo.Name}," +
                            $"字段描述：{filedInfo.Description}");
                    }
                }
                
                // 请求
                var queryResult = await _dataAccessor.DataProvider.QueryTradeHistoryAsync(tradeHistoryParameters);
                foreach(var item in queryResult.Data)
                {
                    Console.WriteLine($"成交时间：{item.DateTime}," +
                        $" 成交价格：{item.LastPrice}," +
                        $"最新交易量：{item.LatestTransactionVolume}");
                }

                IDataSubscription tradeHistorySubscription = null;
                //订阅成交数据
                {
                    tradeHistorySubscription = await _dataAccessor.DataProvider.SubscribeTradeHistoryAsync(tradeHistoryParameters, (object sender, ITradeHistoryResult pushResult) =>
                    {
                        foreach (var item in pushResult.Data)
                        {
                            Console.WriteLine($"成交时间：{item.DateTime}, " +
                                $"成交价格：{item.LastPrice}," +
                                $"最新交易量：{item.LatestTransactionVolume}");
                        }
                    });
                }

                //取消订阅成交数据
                if (tradeHistorySubscription is not null)
                {
                    await tradeHistorySubscription.UnsubscribeAsync();
                }
            }

            ///查行情实时数据
            {
                var fieldIds = _dataAccessor.FieldIds;
                var realtimeQuoteParameters = _dataAccessor.DataProvider.CreateRealtimeQuoteParameters();
                var assetid = _dataAccessor.DataProvider.ToAssetId("USHA600000");
                realtimeQuoteParameters.AssetIds = new[] { assetid };
                realtimeQuoteParameters.FieldIds = new[]
                {
                    fieldIds.AssetName,             //查股票名称
                    fieldIds.LastPrice,             //查最新价
                    fieldIds.PercentageChange,      //查涨跌幅  
                    fieldIds.PreviousClosingPrice,  //查昨收价 
                    fieldIds.StopBrand,             //是否停牌
                };
                realtimeQuoteParameters.TradingSessionType = TradingSessionType.Regular;

                // 查实时数据
                var quoteQueryResult = await _dataAccessor.DataProvider.QueryRealtimeQuoteAsync(realtimeQuoteParameters);
                var fieldValues = quoteQueryResult.ValuesAtAssetId(assetid);
                Console.WriteLine($"股票名称：{fieldValues[fieldIds.AssetName]}," +
                    $"最新价：{fieldValues[fieldIds.LastPrice]}," +
                    $"涨跌幅：{fieldValues[fieldIds.PercentageChange]}");


                //订阅唯一标识
                IDataSubscription subscription = null;
                // 订阅实时数据
                {
                    subscription = await _dataAccessor.DataProvider.SubscribeRealtimeQuoteAsync(realtimeQuoteParameters, (object sender, IRealtimeQuoteResult quoteResult) =>
                    {
                        if (quoteResult != null)
                        {
                            var fieldValue = quoteQueryResult.ValuesAtAssetId(assetid);
                            Console.WriteLine($"股票名称：{fieldValue[fieldIds.AssetName]}," +
                            $"最新价：{fieldValue[fieldIds.LastPrice]}," +
                            $"涨跌幅：{fieldValue[fieldIds.PercentageChange]}");
                        }
                    });
                }

                //取消订阅实时数据
                {
                    if (subscription is not null)
                    {
                        await subscription.UnsubscribeAsync();
                    }
                }
            }

            // 请求当日分时
            {
                var fieldIds = _dataAccessor.FieldIds;
                // 创建请求参数
                var trendQuoteParameters = _dataAccessor.DataProvider.CreateTrendQuoteParameters();
                trendQuoteParameters.AssetId = _dataAccessor.DataProvider.ToAssetId("USHA600000");
                trendQuoteParameters.FieldIds = new[]
                {
                    fieldIds.DateTime,              //查成交时间
                    fieldIds.OpenPrice,             //查开盘价
                    fieldIds.ClosingPrice,          //查收盘价
                    fieldIds.HighestPrice,          //查最高价
                    fieldIds.LowestPrice,           //查最低价
                    fieldIds.TransactionVolume,     //查成交量
                    fieldIds.LastPrice,             //查最新价
                    fieldIds.TransactionAmount,     //查成交金额
                }; ;
                trendQuoteParameters.TradingSessionType = TradingSessionType.Any;

                var trendQuoteResults = await _dataAccessor.DataProvider.QueryIntradayQuoteAsync(trendQuoteParameters);
                
                // 根据查询字段存贮到数组中
                var fullColumnFieldValues = trendQuoteResults.SelectMany(result => result.ColumnFieldValues)
                                .GroupBy(kv => kv.Key)
                                .ToDictionary(group => group.Key, group => group.SelectMany(pair => pair.Value).ToArray());

                for(int i = 0; i < fullColumnFieldValues[fieldIds.DateTime].Length; i++)
                {
                    Console.WriteLine($"成交时间：{fullColumnFieldValues[fieldIds.DateTime][i]}, " +
                        $"开盘价：{fullColumnFieldValues[fieldIds.OpenPrice][i]}," +
                        $"收盘价：{fullColumnFieldValues[fieldIds.ClosingPrice][i]}" +
                        $"最高价：{fullColumnFieldValues[fieldIds.HighestPrice][i]}" +
                        $"最低价：{fullColumnFieldValues[fieldIds.LowestPrice][i]}" +
                        $"成交量：{fullColumnFieldValues[fieldIds.TransactionVolume][i]}" +
                        $"最新价：{fullColumnFieldValues[fieldIds.LastPrice][i]}" +
                        $"成交金额：{fullColumnFieldValues[fieldIds.TransactionAmount][i]}");
                }
            }

            // 请求历史分时
            {
                var fieldIds = _dataAccessor.FieldIds;
          
                // 创建请求参数
                var historicalQuoteParameters = _dataAccessor.DataProvider.CreateHistoricalQuoteParameters();
                historicalQuoteParameters.AssetId = _dataAccessor.DataProvider.ToAssetId("USHA600000");
                historicalQuoteParameters.PriceAdjustment = PriceAdjustment.ExRights;
                historicalQuoteParameters.FieldIds = new[]
                {
                    fieldIds.DateTime,              //查成交时间
                    fieldIds.LastPrice,             //最后成交价
                    fieldIds.TransactionAmount,     //查成交金额
                    fieldIds.TurnoverRate,          //换手率
                    fieldIds.AfterHourVolume,       //盘后量
                    fieldIds.PercentageChange,      //涨跌幅
                };

                var startIndex = 0;
                historicalQuoteParameters.PeriodId = _dataAccessor.PeriodIds.Day;
                historicalQuoteParameters.StartIndex = startIndex;
                historicalQuoteParameters.Count = 50;

                var historicalQueryResult = await _dataAccessor.DataProvider.QueryHistoricalQuoteAsync(historicalQuoteParameters);
                var fullResultFieldValues = historicalQueryResult.FieldValues;
                for (int i = 0; i < fullResultFieldValues[fieldIds.DateTime].Length; i++)
                {
                    Console.WriteLine($"成交时间：{fullResultFieldValues[fieldIds.DateTime][i]}, " +
                        $"最后成交价：{fullResultFieldValues[fieldIds.OpenPrice][i]}," +
                        $"成交金额：{fullResultFieldValues[fieldIds.ClosingPrice][i]}" +
                        $"换手率：{fullResultFieldValues[fieldIds.HighestPrice][i]}" +
                        $"盘后量：{fullResultFieldValues[fieldIds.LowestPrice][i]}" +
                        $"涨跌幅：{fullResultFieldValues[fieldIds.TransactionVolume][i]}");
                }

            }


            // 查询最后一个交易日的所有交易时段
            var sortedFullRanges = await _dataAccessor.DataProvider.QueryLastDayTradingRange(_dataAccessor.DataProvider.ToAssetId("USHA600000"));
            foreach (var period in sortedFullRanges)
            {
                Console.WriteLine($"交易时间段 {period.Begin} - {period.End}");
            }

            //查询支持融资融券股票
            var rzrqBlock = await _dataAccessor.DataProvider.QueryRzrqBlock();
            foreach(var assetid in rzrqBlock)
            {
                Console.WriteLine($"支持融资融券 代码：{assetid.SymbolValue} 市场：{assetid.MarketId}");
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
