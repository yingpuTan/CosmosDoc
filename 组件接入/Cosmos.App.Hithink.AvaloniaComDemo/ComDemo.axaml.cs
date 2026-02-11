using Cosmos.App.Sdk.v1;
using Cosmos.App.Sdk.v1.Primitives;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Protocol;
using Cosmos.DataAccess.v1;
using Cosmos.DataAccess.Trade.HighGate.v1;
using Cosmos.DataAccess.v1.Model;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.Unicode;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Cosmos.App.Hithink.Demo.Shared;
using Avalonia.Threading;
using static Cosmos.App.Hithink.AvaloniaComDemo.AvaloniaComDemoGui;
using PushHandler = ISubscriberRaw<string, string, string, string, string, int>.PushDataHandler;

namespace Cosmos.App.Hithink.AvaloniaComDemo
{
    /// <summary>
    /// Interaction logic for ComDemo.axaml
    /// </summary>
    public partial class ComDemo : UserControl
    {
        private System.Threading.Timer _timer;
        public ComDemo(IDataAccessor dataAccessor, ICosmosAppLogger logger, ICosmosAppContextInjection contextInjection, ITradeDataAccessor tradeDataAccessor)
        {
            InitializeComponent();
            _tradeList = new List<TradeDataAccessor>();
            _SubList = new Dictionary<TradeDataAccessor, ITradeDataSubscription>();
            _dataAccessor = dataAccessor;
            _tradeDataAccessor = tradeDataAccessor;
            _contextInjection = contextInjection;
            _productAccessor = contextInjection.ThisAppContext.GlobalContexts.ProductContext.ProductAccessor;
            _logger = logger;
            _tradeDataAccessor.DataProvider.SubscribAppCommand((object sender, IAppRecommand pushResult) =>
            {
                string content = $"AppGuid：{pushResult.AppGuid}，AppName：{pushResult.AppName}, Content:{pushResult.Content}";
                Dispatcher.UIThread.Post(() =>
                {
                    lab_higate.Content = content;
                });
            });

            //订阅连接是否断开
            _tradeDataAccessor.DataSessionController.TradeStatusChanged += DataSessionController_TradeStatusChanged;

            text_name.TextChanged += (sender, e) =>
            {
                // 在这里处理文本变化的逻辑
                var textBox = sender as TextBox;
                if (textBox != null)
                {
                    foreach (var kvp in dicTextChange)
                    {
                        String strUUid = kvp.Key;
                        var handle = kvp.Value;
                        if (handle != null)
                        {
                            string strSubscriberid = dicTextSub[strUUid];
                            TextChange textPush = new TextChange()
                            {
                                text = textBox.Text ?? string.Empty
                            };
                            /// 触发回调,通知订阅者文本框发生改变
                            handle(strSubscriberid, strUUid, JsonSerializer.Serialize<TextChange>(textPush));
                        }
                    }
                }
            };
            _timer = new System.Threading.Timer(
            callback: TimerCallback,
            state: null,
            dueTime: TimeSpan.FromMilliseconds(1000),
            period: TimeSpan.FromMilliseconds(10)
        );
            _timer.Change(Timeout.Infinite, 0);

        }
        private string _method_type;
        private string _send_type;
        private void TimerCallback(object state)
        {
            var request = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateRequestParameter();
            request.method = "textchanged";
            request.id = _contextInjection.ThisInstanceContext.Id;
            request.param = new JObject()
            {
                ["text1"] = "你好呀1111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text2"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text3"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text4"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text5"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text6"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text7"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text8"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text11"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text12"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text13"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text14"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text15"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text16"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text17"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text18"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text21"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text22"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text23"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text24"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text25"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text26"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text27"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text28"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text31"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text32"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text33"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text34"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text35"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text36"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text37"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",
                ["text38"] = "1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test1111111111111111111111111111111test",

            };
            InvokeType type = _send_type == "broadcast" ? InvokeType.Global : InvokeType.Group;
            if (_method_type == "invoke")
            //发送请求
            {
                var result = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.InvokeWidget(null, type, request, false);

            }
            else
            {
                var result = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.NotifyWidget(null, type, request, false);
            }
            return;
        }
        private void DataSessionController_TradeStatusChanged(object? sender, TradeDataSessionStatus e)
        {
            //已连接
            if (e == TradeDataSessionStatus.Running)
            {
                //处理查询或者重新订阅数据
                _logger?.Log(CosmosLogLevel.Information, "已连接");
            }
            //断开连接
            else if (e == TradeDataSessionStatus.Stopped)
            {
                _logger?.Log(CosmosLogLevel.Information, "已断开");

            }
        }


        public void SetWebContent(object obj)
        {
            web_demo.Content = obj;
        }

        /// <summary>
        /// 获取文本信息
        /// </summary>
        /// <returns>文本输入框内容</returns>
        public String GetText()
        {
            return text_name.Text ?? string.Empty;
        }

        /// <summary>
        /// 订阅文本变化信息
        /// </summary>
        /// <param name="pushDataHandler">订阅者回调函数</param>
        /// <returns></returns>
        public String SubscribeTextChange(string subscriberId, PushHandler pushDataHandler)
        {
            String strUUid = Guid.NewGuid().ToString();
            dicTextChange[strUUid] = pushDataHandler;
            dicTextSub[strUUid] = subscriberId;
            return strUUid;
        }

        /// <summary>
        /// 取消订阅
        /// </summary>
        /// <param name="subscribeId">订阅时返回的uuid</param>
        public void UnSubscribeTextChange(string subscriptionId)
        {
            dicTextChange.Remove(subscriptionId);
            dicTextSub.Remove(subscriptionId);
        }
        /// <summary>
        /// 设置文本信息
        /// </summary>
        /// <param name="str">需要设置的字符串信息</param>
        public void SetText(string str)
        {
            text_name.Text = str;
        }
        private void btn_get_Click(object? sender, RoutedEventArgs e)
        {
            Task.Factory.StartNew(async () =>
            {
                var queryResult = await _dataAccessor.DataProvider.QueryWatchListAsync();
                if (queryResult != null)
                {
                    String strSelfSotck = String.Join(',', queryResult.AssetIds);
                    _logger?.Log(CosmosLogLevel.Information, $"获取到自选股列表：{strSelfSotck}");
                    Dispatcher.UIThread.Post(() =>
                    {
                        text_selfstock.Text = strSelfSotck;
                    });
                }
            });
        }

        /// <summary>
        /// 主动调用宿主方法，但是宿主是否处理需要宿主开发人员响应
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void btn_send_msg_Click(object? sender, RoutedEventArgs e)
        {
            ///获取当前实例唯一标识
            var cosmosOperator = Convert.ToString(_contextInjection.ThisInstanceContext.InstanceId);
            TextChange textInvoke = new TextChange()
            {
                text = text_name.Text ?? string.Empty
            };
            //向宿主调用comTestSetText方法,需要宿主实现该方法
            string result = await _productAccessor.InvokeAsync(cosmosOperator, "comTestSetText", JsonSerializer.Serialize<TextChange>(textInvoke));
            AvaloniaToast.Show("发送完成");
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void btn_sub_higate_Click(object? sender, RoutedEventArgs e)
        {
            /// 调用第三方订阅接口
            ISubscribeParameters subscribeParameters = _tradeDataAccessor.DataProvider.CreateSubscribeParameters();
            subscribeParameters.Action = "subscribe.plugin.test";
            subscribeParameters.Server = "test_server1";
            subscribeParameters.Parameters = "date=today";
            subscribeParameters.Topic = "entrust";
            subscribeParameters.uuid = Guid.NewGuid().ToString();
            pushsubscribetion_ = await _tradeDataAccessor.DataProvider.SubscribThirdModule(subscribeParameters, (object sender, IPushData pushResult) =>
            {
                _logger?.Log(CosmosLogLevel.Information, $" RecvPushData :{pushResult.data}");
            });

            if (pushsubscribetion_.code != 0)
            {
                AvaloniaToast.Show($"订阅失败 msg:{pushsubscribetion_.msg}", CosmosLogLevel.Error);
                _logger?.Log(CosmosLogLevel.Error, $"订阅失败 msg:{pushsubscribetion_.msg}");
            }
            else
            {
                AvaloniaToast.Show("订阅higate完成");
                _logger?.Log(CosmosLogLevel.Information, $"订阅higate成功 code:{pushsubscribetion_.code}, msg:{pushsubscribetion_.msg}");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void btn_unsub_higate_Click(object? sender, RoutedEventArgs e)
        {
            if (pushsubscribetion_ != null)
            {
                /// 调用第三方取消订阅接口
                IUnSubscribeParameters unsubscribeParameters = _tradeDataAccessor.DataProvider.CreateUnSubscribeParameters();
                unsubscribeParameters.Action = "unsubscribe.plugin.test";
                unsubscribeParameters.Server = "test_server1";
                unsubscribeParameters.Parameters = "topic=entrust,date=today";
                unsubscribeParameters.Topic = "entrust";
                unsubscribeParameters.uuid = Guid.NewGuid().ToString();

                var result = await _tradeDataAccessor.DataProvider.UnSubscribThirdModule(unsubscribeParameters, pushsubscribetion_);
                if (result.code == 0)
                {
                    AvaloniaToast.Show("取消订阅higate成功");
                    _logger?.Log(CosmosLogLevel.Information, "取消订阅higate成功");
                }
                else
                {
                    AvaloniaToast.Show($"取消订阅失败 msg:{result.msg}", CosmosLogLevel.Error);
                    _logger?.Log(CosmosLogLevel.Error, $"取消订阅失败 msg:{result.msg}");

                }
            }
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void btn_req_higate_Click(object? sender, RoutedEventArgs e)
        {
            /// 调用第三方接口
            IThirdRequestParameters requestParameters = _tradeDataAccessor.DataProvider.CreateThirdRequestParameters();
            requestParameters.Action = "request.plugin.test";
            requestParameters.Server = "test_server1";
            requestParameters.uuid = Guid.NewGuid().ToString();
            var options = new JsonSerializerOptions
            {
                Encoder = JavaScriptEncoder.Create(
                    allowedRanges: new[]
                    {
                        UnicodeRanges.BasicLatin,      // ASCII 字符
                        UnicodeRanges.CjkUnifiedIdeographs // 中文字符（CJK 统一表意文字）
                    }
                )
            };
            JsonObject context = new JsonObject();
            context["name"] = text_higate.Text ?? string.Empty;
            requestParameters.Parameters = JsonSerializer.Serialize(context, options);

            //异步不卡住界面
            {
                await _tradeDataAccessor.DataProvider.ThirdModuleRequestAsync(requestParameters, (object sender, IThirdResponse result) =>
                {
                    _logger?.Log(CosmosLogLevel.Information, $"reqresult:{result.data} id : {result.uuid} code:{result.code}  msg:{result.msg}");
                });
            }

        }

        // 组件快捷键注册处理示例
        private void Button_Click(object? sender, RoutedEventArgs e)
        {
            if (String.IsNullOrEmpty(text_hotkey.Text))
            {
                AvaloniaToast.Show("请输入快捷键", CosmosLogLevel.Warning);
            }

            //接受全局快捷键注册通知
            _contextInjection.ThisAppContext.GlobalContexts.
                EngineContext.BusinessRequest.ShortcutManager.HotKeyRegist += (s, v) =>
                {
                    foreach (var result in v)
                    {
                        if (result.IsSuccessful)
                        {
                            Console.WriteLine($"组件快捷键注册成功 id：{result.ShortcutId}  描述：{result.Description}  快捷键：{result.ShortCut}");
                        }
                        else
                        {
                            Console.WriteLine($"组件快捷键注册失败 reason：{result.ErrorMessage}");
                        }
                    }
                };

            //接受全局快捷键取消注册通知
            _contextInjection.ThisAppContext.GlobalContexts.
                EngineContext.BusinessRequest.ShortcutManager.HotKeyUnRegist += (s, v) =>
                {
                    foreach (var result in v)
                    {
                        if (result.IsSuccessful)
                        {
                            Console.WriteLine($"快捷键取消注册成功 id：{result.ShortcutId}");
                        }
                    }
                };

            // 获取全局快捷键
            IReadOnlyDictionary<string, RegisteredShortcut> global = _contextInjection.ThisAppContext.GlobalContexts.
                EngineContext.BusinessRequest.ShortcutManager.GetGlobalShortcutAsync().Result;
            text_hotkey.Text = Newtonsoft.Json.JsonConvert.SerializeObject(global);

            // 内部快捷键响应处理

            // 通知框架注册组件快捷键
            var list = new List<ShortcutRegistrationRequest>();
            list.Add(new ShortcutRegistrationRequest
            {
                // 组件快捷键唯一标识
                ShortcutId = "1",
                ShortCut = new HotKey
                {
                    // windows辅助键
                    // MOD_ALT      0x0001
                    // MOD_CONTROL  0x0002
                    // MOD_NOREPEAT 0x4000
                    // MOD_SHIFT    0x0004
                    // MOD_WIN      0x0008
                    Modifiers = 1,

                    // 虚拟按键映射 参照windows Virtual-key-codes
                    Key = 66,
                }
            });

            var result = _contextInjection.ThisAppContext.GlobalContexts.
                EngineContext.BusinessRequest.ShortcutManager.RegisterComponentShortcutAsync(_contextInjection.ThisInstanceContext.WidgetGuid.ToString(), list);
        }


        private void btn_gettheme_Click(object? sender, RoutedEventArgs e)
        {
            ///获取当前颜色主题
            var theme = _contextInjection.ThisAppContext.GlobalContexts.VisualContext.ColorScheme;
            text_theme.Text = theme.ToString();
        }

        private void btn_getthemedic_Click(object? sender, RoutedEventArgs e)
        {
            ///获取当前主题的资源字典
            var dictionary = _contextInjection.ThisAppContext.GlobalContexts.VisualContext.ThemeResources;
            var text = Newtonsoft.Json.JsonConvert.SerializeObject(dictionary);
            _logger?.Log(CosmosLogLevel.Information, $"ThemeResources:{text}");
            text_theme.Text = text;
        }

        /// <summary>
        /// 行情数据访问器
        /// </summary>
        private IDataAccessor _dataAccessor { get; set; }

        private ITradeDataAccessor _tradeDataAccessor { get; set; }


        /// <summary>
        /// 日志记录器
        /// </summary>
        private ICosmosAppLogger _logger { get; set; }

        /// <summary>
        /// 宿主访问器
        /// </summary>
        public ICosmosProductAccessor _productAccessor { get; set; }

        /// <summary>
        /// 该接口定义了当前组件和实例上下文的注入点，这些上下文在构造后由Cosmos引擎自动注入
        /// 可在该成员中实现对上下文的初始化和修改
        /// </summary>
        public ICosmosAppContextInjection _contextInjection { get; set; }

        /// <summary>
        /// 保存textchange订阅者回调函数
        /// </summary>
        private Dictionary<string, PushHandler> dicTextChange = new Dictionary<string, PushHandler>();
        private Dictionary<string, string> dicTextSub = new Dictionary<string, string>();

        ITradeDataSubscription pushsubscribetion_;

        private void btn_sendglobal_Click(object? sender, RoutedEventArgs e)
        {
            var request = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateRequestParameter();
            request.method = "textchanged";
            request.id = _contextInjection.ThisInstanceContext.Id;
            request.param = new JObject()
            {
                ["text"] = text_sendcom.Text ?? string.Empty
            };
            //发送请求
            if (comuType.SelectedIndex == 0)
            {
                var result = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.InvokeWidget(null, InvokeType.Global, request, false);
            }
            //发送通知
            else
            {
                _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.NotifyWidget(null, InvokeType.Global, request, false);
            }
        }

        private void btn_sendcom_Click(object? sender, RoutedEventArgs e)
        {
            var request = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.CreateRequestParameter();
            request.method = "textchanged";
            request.id = _contextInjection.ThisInstanceContext.Id;
            request.param = new JObject()
            {
                ["text"] = text_sendcom.Text ?? string.Empty
            };

            //发送请求
            if (comuType.SelectedIndex == 0)
            {
                var result = _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.InvokeWidget(text_sender.Text, InvokeType.Group, request, false);
                result.ContinueWith(t =>
                {
                    var result = t.Result;
                    var txt = $"AvaloniaComDemoGui 接收到其他组件返回的invoke ,代码：{result.code} ，参数为 {result.result.ToString()}, 来源{result.id}";
                    ReceiveTxt(txt);

                });
            }
            //发送通知
            else
            {
                _contextInjection.ThisAppContext.GlobalContexts.EngineContext.BusinessRequest.NotifyWidget(text_sender.Text, InvokeType.Group, request, false);
            }
        }

        public void ReceiveTxt(string str)
        {
            string str_time = DateTime.Now.ToString() + ":";
            if (string.IsNullOrEmpty(str))
            {
                str_time = "";
            }
            Dispatcher.UIThread.Post(() =>
            {
                text_receive_content.Text += str_time + str + Environment.NewLine;
            });
        }
        private void btn_sendinstance_Click(object? sender, RoutedEventArgs e)
        {

        }

        private void comuType_SelectionChanged(object? sender, SelectionChangedEventArgs e)
        {
            _method_type = (comuType.SelectedIndex == 0) ? "invoke" : "notify";
        }

        private void check_send_type_Checked(object? sender, RoutedEventArgs e)
        {
            bool? is_checked = false;
            is_checked = ((CheckBox)sender).IsChecked; // 获取当前状态
            if (is_checked == true)
            {
                _send_type = "broadcast";
            }
            else
            {
                _send_type = "group";
            }

        }

        private Thread timerThread;
        private volatile bool isRunning = false;

        [DllImport("winmm.dll")]
        static extern uint timeBeginPeriod(uint period);

        [DllImport("winmm.dll")]
        static extern uint timeEndPeriod(uint period);
        private void check_timer_Checked(object? sender, RoutedEventArgs e)
        {
            ReceiveTxt("");

            int interval = 1000;
            int.TryParse(timer_interval.Text, out interval);

            int total = 5000;
            int.TryParse(total_count.Text, out total);

            bool? is_checked = false;
            is_checked = ((CheckBox)sender).IsChecked; // 获取当前状态
            if (is_checked == true)
            {
                isRunning = true;
                timerThread = new Thread(() =>
                {
                    while (isRunning && (total > 0 || total < 0))
                    {
                        {
                            TimerCallback(null);
                            total--;
                            if (total == 0)
                            {
                                ReceiveTxt("本次发送完成");
                                break;
                            }
                        }
                        timeBeginPeriod(1);
                        System.Threading.Thread.Sleep(interval);
                        timeEndPeriod(1);
                    }
                });
                timerThread.IsBackground = true; // 设置为后台线程，确保主线程退出时线程也会终止
                timerThread.Start();
            }
            else
            {
                _timer.Change(Timeout.Infinite, interval);

                isRunning = false;
                if (_timer != null)
                {
                    _timer.Change(Timeout.Infinite, interval);
                }
            }
        }

        public IList<TradeDataAccessor> _tradeList;
        public IDictionary<TradeDataAccessor, ITradeDataSubscription> _SubList;
        private async void btn_create_Click(object? sender, RoutedEventArgs e)
        {
            int linkCount = int.Parse(text_Link.Text ?? "0");
            int account = int.Parse(text_Name.Text ?? "0");
            var baseInfo = await _tradeDataAccessor.DataProvider.GetBaseInfo();
            for (int i = 0; i < linkCount; i++)
            {
                HighGateProductInfo ProductInfo = new HighGateProductInfo()
                {
                    ProductID = baseInfo.ProductID,
                    Account = (account + i).ToString(),
                    Ip = baseInfo.Ip,
                    Port = baseInfo.Port,

                };

                var traderDataAccessor = new TradeDataAccessor(null, ProductInfo);
                await traderDataAccessor.DataSessionController.StartAsync();
                _tradeList.Add(traderDataAccessor);
            }
        }

        private System.Threading.Timer _reqTimer;

        private async void ReqTimerCallback(object? state)
        {
            try
            {
                /// 调用第三方接口
                IThirdRequestParameters requestParameters = _tradeDataAccessor.DataProvider.CreateThirdRequestParameters();
                requestParameters.Action = "request.plugin.test";
                requestParameters.Server = "test_server1";
                var options = new JsonSerializerOptions
                {
                    Encoder = JavaScriptEncoder.Create(
                        allowedRanges: new[]
                        {
                        UnicodeRanges.BasicLatin,      // ASCII 字符
                        UnicodeRanges.CjkUnifiedIdeographs // 中文字符（CJK 统一表意文字）
                        }
                    )
                };
                JsonObject context = new JsonObject();
                context["name"] = "123123";
                requestParameters.Parameters = JsonSerializer.Serialize(context, options);

                foreach (var tradeAccessor in _tradeList)
                {
                    requestParameters.uuid = Guid.NewGuid().ToString();
                    await tradeAccessor.DataProvider.ThirdModuleRequestAsync(requestParameters, (object sender, IThirdResponse result) =>
                    {
                        _logger?.Log(CosmosLogLevel.Information, $"reqresult:id : {result.uuid} code:{result.code}  msg:{result.msg}");
                    });
                }
            }
            catch (Exception ex)
            {
                // 异常处理
                Console.WriteLine($"定时器异常: {ex.Message}");
            }
        }

        private async void btn_req_Click(object? sender, RoutedEventArgs e)
        {
            // 创建定时器（初始延迟1秒，间隔3000毫秒）
            _reqTimer = new System.Threading.Timer(
               callback: ReqTimerCallback,
               state: null,
               dueTime: TimeSpan.FromMilliseconds(1000),
               period: TimeSpan.FromMilliseconds(3000)
           );

        }

        private async void btn_sub_Click(object? sender, RoutedEventArgs e)
        {

            /// 调用第三方订阅接口
            ISubscribeParameters subscribeParameters = _tradeDataAccessor.DataProvider.CreateSubscribeParameters();
            subscribeParameters.Action = "subscribe.plugin.test";
            subscribeParameters.Server = "test_server1";
            subscribeParameters.Parameters = "date=today";
            subscribeParameters.Topic = "entrust";

            foreach (var tradeAccessor in _tradeList)
            {
                subscribeParameters.uuid = Guid.NewGuid().ToString();
                var subscribetion = await tradeAccessor.DataProvider.SubscribThirdModule(subscribeParameters, (object sender, IPushData pushResult) =>
                {
                    _logger?.Log(CosmosLogLevel.Information, $" RecvPushData :");
                });

                if (subscribetion.code != 0)
                {
                    AvaloniaToast.Show($"订阅失败 msg:{subscribetion.msg}", CosmosLogLevel.Error);
                    _logger?.Log(CosmosLogLevel.Error, $"订阅失败 msg:{subscribetion.msg}");
                }
                else
                {
                    _SubList[tradeAccessor] = subscribetion;
                    AvaloniaToast.Show("订阅higate成功");
                    _logger?.Log(CosmosLogLevel.Information, "订阅higate成功");
                }
            }

        }

        private async void btn_ubsub_Click(object? sender, RoutedEventArgs e)
        {
            /// 调用第三方取消订阅接口
            IUnSubscribeParameters unsubscribeParameters = _tradeDataAccessor.DataProvider.CreateUnSubscribeParameters();
            unsubscribeParameters.Action = "unsubscribe.plugin.test";
            unsubscribeParameters.Server = "test_server1";
            unsubscribeParameters.Parameters = "topic=entrust,date=today";
            unsubscribeParameters.Topic = "entrust";
            foreach (var sub in _SubList)
            {
                unsubscribeParameters.uuid = Guid.NewGuid().ToString();
                var result = await sub.Key.DataProvider.UnSubscribThirdModule(unsubscribeParameters, sub.Value);
                if (result.code == 0)
                {
                    AvaloniaToast.Show("取消订阅higate成功");
                    _logger?.Log(CosmosLogLevel.Information, "取消订阅higate成功");
                }
                else
                {
                    AvaloniaToast.Show($"取消订阅 msg:{result.msg}", CosmosLogLevel.Error);
                    _logger?.Log(CosmosLogLevel.Error, $"取消订阅 msg:{result.msg}");
                }
            }
        }

        public async Task stop()
        {
            _reqTimer?.Change(Timeout.Infinite, 0);
            _reqTimer?.Dispose();
            foreach (var tradeAccessor in _tradeList)
            {
                await tradeAccessor.DataSessionController.StopAsync();
            }
        }
    }
}

