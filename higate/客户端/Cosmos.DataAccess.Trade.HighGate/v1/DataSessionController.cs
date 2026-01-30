using Cosmos.DataAccess.Trade.HighGate.Internals;
using Cosmos.DataAccess.Trade.HighGate.Internals.Protocol;
using Cosmos.DataAccess.Trade.HighGate.v1.Protocol;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using Cosmos.Observability.Application.Contracts.Logger;
using Microsoft.Extensions.Caching.Memory;
using Microsoft.VisualBasic;
using Newtonsoft.Json.Linq;
using Serilog.Parsing;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.ServiceProcess;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;

namespace Cosmos.DataAccess.Trade.HighGate.v1
{
    public class HighGateProductInfo : IBaseLoginInfo
    {
        /// <summary>
        /// 产品名
        /// </summary>
        public string ProductID { get; set; } = "";

        /// <summary>
        /// 账户名
        /// </summary>
        public string Account { get; set; } = "";

        /// <summary>
        /// 密码
        /// </summary>
        public string PassWord { get; set; } = "";

        /// <summary>
        /// 本次登录token
        /// </summary>
        public string Token { get; set; } = "";

        /// <summary>
        /// 消息中心ip
        /// </summary>
        public string Ip { get; set; } = "10.4.123.112";

        /// <summary>
        /// 消息中心端口
        /// </summary>
        public int Port { get; set; } = 9999;
    }

    /// <summary>
    /// 会话状态
    /// </summary>
    public enum DataSessionStatus
    {
        /// <summary>
        /// 已停止
        /// </summary>
        Stopped = 1,

        /// <summary>
        /// 启动中
        /// </summary>
        StartPending = 2,

        /// <summary>
        /// 停止中
        /// </summary>
        StopPending = 3,

        /// <summary>
        /// 运行中
        /// </summary>
        Running = 4,

        /// <summary>
        /// 唤醒中
        /// </summary>
        ContinuePending = 5,

        /// <summary>
        /// 暂停中
        /// </summary>
        PausePending = 6,

        /// <summary>
        /// 已暂停
        /// </summary>
        Paused = 7
    }

    public class HigateMessageData
    {
        /// <summary>
        /// 请求类型
        /// </summary>
        public ReqType Action { get; set; }

        /// <summary>
        /// 消息内容
        /// </summary>
        public string Content { get; set; }
    }

    /// <summary>
    /// 会话器
    /// </summary>
    public class DataSessionController : IDataSessionController
    {
        //创建session
        [DllImport("LHsession.dll")]
        internal static extern IntPtr CreateMsgCenterApi(IntPtr cb);

        //登录
        [DllImport("LHsession.dll", CharSet = CharSet.Ansi)]
        internal static extern void Login(IntPtr sdkHandle, byte[] address, int port, byte[] account, byte[] password, bool main);

        //登录
        [DllImport("LHsession.dll", CharSet = CharSet.Ansi)]
        internal static extern int Send(IntPtr sdkHandle, byte[] sendstr, int len);

        public DataSessionController(ICosmosLogger cosmosLogger, HighGateProductInfo vendorProductInfo)
        {
            this.cosmosLogger = cosmosLogger;
            TradeStatus = TradeDataSessionStatus.Stopped;
            _vendorProductInfo = vendorProductInfo;
        }

        /// <summary>
        /// 设置账号和token信息
        /// </summary>
        /// <returns></returns>
        public Task SetAccountToken(string account, string token, string password)
        {
            _vendorProductInfo.Account = account;
            _vendorProductInfo.Token = token;
            _vendorProductInfo.PassWord = password;
            return Task.CompletedTask;
        }

        /// <summary>
        /// 保存订阅信息
        /// </summary>
        /// <param name="topic"></param>
        /// <returns></returns>
        public DataPushJob SubScribe(string topic)
        {
            DataSubscription subscription = new DataSubscription()
            {
                Topic = topic,
            };

            DataPushJob dataPushJob = new DataPushJob()
            {
                Subscription = subscription,
                Session = this,
            };

            subscription.PushJob = dataPushJob;
            ChangeSubscription(dataPushJob.Subscription, true, null);
            return dataPushJob;
        }
        /// <summary>
        /// 启动会话
        /// </summary>
        /// <returns></returns>
        public Task StartAsync(bool bMain = false)
        {
            DoConnect(bMain);

            return Task.CompletedTask;
        }

        /// <summary>
        /// 停止会话
        /// </summary>
        /// <returns></returns>
        public Task StopAsync()
        {
            if(TradeStatus == TradeDataSessionStatus.Running)
            {
                JObject jsParameter = new JObject();
                jsParameter["action"] = "logout";
                jsParameter["token"] = _vendorProductInfo.Token;
                DataRequest request = new DataRequest()
                {   
                    uuid = Guid.NewGuid().ToString(),
                    method = "client",
                    servicename = "",
                    parameters = jsParameter,
                    token = _vendorProductInfo.Token
                };

                var strRequest = request.Serialize();
                Send(sdkHandle, strRequest, strRequest.Length);
            }
           
            return Task.CompletedTask;
        }
        private void _raiseStatusChanged()
        {
            TradeStatusChanged?.Invoke(this, TradeStatus);
        }
        private static IntPtr DelegateToFunction<DelegateType>(DelegateType d)
        {
            return Marshal.GetFunctionPointerForDelegate<DelegateType>(d);
        }

        /// <summary>
        /// 第三方模块请求
        /// </summary>
        /// <param name="parameters"></param>
        /// <returns></returns>
        public async Task<IThirdResponse> ThirdRequest(IThirdRequestParameters parameters, bool bSub = false)
        {
            Guid result;
            if(string.IsNullOrEmpty(parameters?.uuid) || !Guid.TryParse(parameters?.uuid, out result))
            {
                var response = new ThirdRequestResult()
                {
                    code = 303,
                    msg = "uuid 不能为空",
                };

                return response;
            }

            //订阅和取消订阅请求topic不能为空
            if(bSub && string.IsNullOrEmpty(parameters.Topic))
            {
                var response = new ThirdRequestResult()
                {
                    uuid = parameters.uuid,
                    code = 304,
                    msg = "topic不能为空",
                };
                return response;
            }
            else if(!bSub && !string.IsNullOrEmpty(parameters.Topic))
            {
                var response = new ThirdRequestResult()
                {
                    uuid = parameters.uuid,
                    code = 305,
                    msg = "请求不能填入topic",
                };
                return response;
            }

            JObject jsParameter = new JObject();
            jsParameter["user"] = _account;
            jsParameter["type"] = parameters.Action;
            jsParameter["data"] = parameters.Parameters;
            jsParameter["uuid"] = parameters.uuid;
            jsParameter["topic"] = parameters.Topic;
            DataRequest request = new DataRequest();

            request.uuid = parameters.uuid;
            request.method = "plugin_data";
            request.servicename = parameters.Server;
            request.parameters = jsParameter;
            request.token = _vendorProductInfo.Token;

            DataResponse dataResponse = await SendRequest(request);
            if (dataResponse.IsBad)
            {
                var errorResponse = JsonSerializer.Deserialize<ErrorResponse>(dataResponse.response);
                var thirdQueryResult = new ThirdRequestResult()
                {
                    code = errorResponse.code,
                    msg = errorResponse.message,
                    uuid = dataResponse.uuid
                };
                return thirdQueryResult;
            }
            else
            {
                var thirdQueryResult = JsonSerializer.Deserialize<ThirdRequestResult>(dataResponse.response);
                if(thirdQueryResult is not null)
                {
                    thirdQueryResult.uuid = dataResponse.uuid;
                    thirdQueryResult.sendtime = dataResponse.sendtime;
                    thirdQueryResult.recvtime = dataResponse.recvtime;
                }
                return thirdQueryResult;
            }
        }

        /// <summary>
        /// 第三方模块请求
        /// </summary>
        /// <param name="parameters"></param>
        /// <returns></returns>
        public async Task ThirdRequestAsync(IThirdRequestParameters parameters, EventHandler<IThirdResponse> callback)
        {
            await Task.Run(async () =>
            {
                var response = await ThirdRequest(parameters);
                callback?.Invoke(this, response);
            });
        }
        /// <summary>
        /// 第三方模块订阅
        /// </summary>
        /// <param name="subscriberParameters"></param>
        /// <returns></returns>
        public async Task<DataPushJob> ThirdSubsribe(ISubscribeParameters subscriberParameters)
        {
            var response = await ThirdRequest(subscriberParameters, true);
            //订阅成功、保存订阅列表
            if (response.code == 0)
            {
                var dataPushJob = SubScribe(subscriberParameters.Topic);
                dataPushJob.IsBad = false;
                dataPushJob.code = response.code;
                dataPushJob.msg = response.msg;
                return dataPushJob;
            }
            else
            {
                var datapush = new DataPushJob()
                {
                    IsBad = true,
                    code = response.code,
                    msg = response.msg,
                };
                return datapush;
            }
        }

        /// <summary>
        /// 第三方取消订阅
        /// </summary>
        /// <param name="subscriberParameters"></param>
        /// <returns></returns>
        public async Task<IUnSubscribeResult> ThirdUnSubsribe(IUnSubscribeParameters unsubscriberParameters, ITradeDataSubscription subscription)
        {
           return await subscription?.UnsubscribeAsync(unsubscriberParameters);
        }

        /// <summary>
        /// 发送请求
        /// </summary>
        /// <param name="request"></param>
        /// <returns></returns>
        public async Task<DataResponse> SendRequest(DataRequest request)
        {
            var response = new DataResponse();
            if (sdkHandle.ToInt32() > 0)
            {
                //连接已建立，可发送消息
                if (TradeStatus == TradeDataSessionStatus.Running)
                {
                    AddActiveRequest(request);
                    var strSend = request.Serialize();
                    cosmosLogger?.Log(2, $"Higate send: {Encoding.UTF8.GetString(strSend)}");
                    request.Status.BeforeSendDateTime = DateTime.Now;
                    response.sendtime = request.Status.BeforeSendDateTime;

                    int code = Send(sdkHandle, strSend, strSend.Length);
                    request.Status.AfterSendDateTime = DateTime.Now;
                    if (code != 0 || !request.ResponsedEvent.Wait(60000))
                    {
                        response.recvtime = DateTime.Now;
                        RemoveActiveRequest(request);
                        ErrorResponse error = new ErrorResponse();
                        if (code != 0)
                        {
                            error.code = code;
                            error.message = "解析请求串失败";
                            cosmosLogger?.Log(2, $"request parse failed send: {strSend}, length = {strSend.Length}");
                        }
                        else
                        {
                            error.code = 300;
                            error.message = "请求超时";
                            response.IsTimeOutResponse = true;
                            cosmosLogger?.Log(2, $"request timeout: {strSend}, length = {strSend.Length}");

                        }
                        response.uuid = request.uuid;
                        response.response = JsonSerializer.Serialize(error);
                        response.IsBad = true;
                        request.Status.Response = response;
                    }
                    else
                    {
                        response.recvtime = DateTime.Now;
                        request.ResponsedEvent.Dispose();// 释放句柄
                    }
                    return request.Status.Response;
                }
                else
                {
                    response.recvtime = DateTime.Now;
                    response.uuid = request.uuid;
                    ErrorResponse error = new ErrorResponse();
                    error.code = 302;
                    error.message = "连接不成功或者已断开，请稍后再试";
                    response.IsBad = true;
                    response.response = JsonSerializer.Serialize(error);
                    cosmosLogger?.Log(2, $"request disconnect");
                    return response;
                }
            }
            else
            {
                response.uuid = request.uuid;
                ErrorResponse error = new ErrorResponse();
                error.code = 301;
                error.message = "未连接";
                response.IsBad = true;
                response.response = JsonSerializer.Serialize(error);
                cosmosLogger?.Log(2, $"request unconnect");
                return response;
            }
        }

        // 消息回调函数
        public delegate void CallbackDelegate(IntPtr sdkHandle, IntPtr result, int len);

        /// <summary>
        /// 接收到消息
        /// </summary>
        /// <param name="sdkHandle"></param>
        /// <param name="result"></param>
        /// <param name="len"></param>
        private async void CallBack(IntPtr sdkHandle, IntPtr result, int len)
        {
            // 将 IntPtr 转换为字节数组
            byte[] gbkBytes = new byte[len];
            Marshal.Copy(result, gbkBytes, 0, len);

            // 将 GBK 字节数组解码为 UTF-8 字符串
            string gbkString = Encoding.GetEncoding("GBK").GetString(gbkBytes);
            string utf8String = Encoding.UTF8.GetString(Encoding.Convert(Encoding.GetEncoding("GBK"), Encoding.UTF8, gbkBytes));

            cosmosLogger?.Log(2, $"Recv CallBack Msg: {utf8String}");
            var highGateMsgData = JsonSerializer.Deserialize<HigateMessageData>(utf8String);
            if(String.IsNullOrEmpty(highGateMsgData.Content)) return;

            try
            {
                JsonDocument document = JsonDocument.Parse(highGateMsgData.Content);
                DataResponse dataResponse = new DataResponse();
                dataResponse.recvtime = DateTime.Now;
                if (document.RootElement.TryGetProperty("id", out var id))
                {
                    dataResponse.uuid = id.GetString();

                    if (document.RootElement.TryGetProperty("result", out var results))
                    {
                        dataResponse.response = results.GetRawText();
                    }
                    else
                    {
                        dataResponse.IsBad = true;
                        dataResponse.response = document.RootElement.GetProperty("error").GetRawText();
                    }
                }
                //处理推送
                else
                {
                    if (document.RootElement.TryGetProperty("method", out var method))
                    {
                        dataResponse.Topic = method.GetString();
                    }

                    if (document.RootElement.TryGetProperty("params", out var param))
                    {
                        //topic为topic.pluginmanage.push时，表示对接第三方服务的推送，需要重新解析一下param中的参数，获取真正的topic
                        if (dataResponse.Topic == "topic.pluginmanage.push")
                        {
                            dataResponse.Topic = param.GetProperty("topic").GetString();
                            dataResponse.response = param.GetProperty("msg").GetString();
                        }
                        else
                        {
                            dataResponse.response = param.GetRawText();
                        }
                    }
                }

                if (highGateMsgData == null) return;
                switch (highGateMsgData.Action)
                {
                    /// 单独处理登录请求
                    case ReqType.RT_SESSION_USER_LOGIN:
                        await HandleLogin(dataResponse);
                        break;
                    default:
                        await HandleResponse(dataResponse);
                        break;
                }
            }
            catch (Exception ex)
            {
                cosmosLogger?.Log(2, $"Recv CallBack Msg ex: {ex.ToString()}");
                return;
            }

        }

        private async Task HandleResponse(DataResponse dataResponse)
        {
            if (dataResponse == null) return;

            ///推送数据
            if(string.IsNullOrEmpty(dataResponse.uuid))
            {
                await HandlePushMsg(dataResponse);
            }
            ///应答数据
            else
            {
                // 根据Response查找Request
                var req = FindActiveRequest(dataResponse);
                if (req == null)
                {
                    return;
                }
                dataResponse.sendtime = req.Status.BeforeSendDateTime;
                // 从Active队列里移除Request
                RemoveActiveRequest(req);

                // 请求包
                req.Status.Response = dataResponse;
                req.ResponsedEvent.Set();
            }
        }
        private async Task HandleLogin(DataResponse dataResponse)
        {
            if (dataResponse == null) return;

            if (dataResponse.IsBad) return;


            var loginResult = JsonSerializer.Deserialize<LoginResult>(dataResponse.response);
            if (loginResult == null) return;

            _vendorProductInfo.Token = loginResult.token;
            TradeStatus = TradeDataSessionStatus.Running;
        }

        private async Task HandlePushMsg(DataResponse data_response)
        {
            if(data_response == null) return;
            if(data_response.IsBad) return;

            //连接状态推送信息
            if(data_response.Topic == "Session_Event")
            {
                JsonDocument document = JsonDocument.Parse(data_response.response);
                if(document.RootElement.TryGetProperty("event", out var events))
                {
                    //连接已断开
                    if(events.ToString() == SessionEvent.DISCONNECT)
                    {
                        TradeStatus = TradeDataSessionStatus.Stopped;
                    }
                }
            }

            List<DataPushJob> push_jobs_shadow = new List<DataPushJob>();

            lock (ActivePushJobLock)
            {
                // 找到对应的订阅
                var push_jobs = FindActivePushJobs(data_response);

                if (push_jobs == null)
                {
                    return;
                }
                // 触发其push_job的推送事件

                push_jobs_shadow.AddRange(push_jobs);
            }

            foreach (var push_job in push_jobs_shadow)
            {
                push_job.RaisePushedEvent(data_response);
            }

        }

        private static Dictionary<IntPtr, DataSessionController> _mapCenter = new Dictionary<nint, DataSessionController>();

        private CallbackDelegate _callback;

        private IntPtr _truePtr;

        public void DoConnect(bool bMain = false)
        {
            _account = _vendorProductInfo.Account + "_" + _vendorProductInfo.ProductID;
            string password = _vendorProductInfo.PassWord + "||" + _vendorProductInfo.Token;
            _callback = new CallbackDelegate(CallBack);
            _truePtr = DelegateToFunction(_callback);
            sdkHandle = CreateMsgCenterApi(_truePtr);
            if (sdkHandle.ToInt32() > 0)
            {
                _mapCenter[sdkHandle] = this;
                var ip = Encoding.GetEncoding("GBK").GetBytes(_vendorProductInfo.Ip);
                var account = Encoding.GetEncoding("GBK").GetBytes(_account);
                var pass = Encoding.GetEncoding("GBK").GetBytes(password);
                Login(sdkHandle, ip, _vendorProductInfo.Port, account, pass, bMain);
            }
        }

        public IBaseLoginInfo GetBaseInfo()
        {
            return _vendorProductInfo;
        }

        private IntPtr sdkHandle = new IntPtr(0);

        /// <summary>
        /// 会话状态变更事件
        /// </summary>
        public event EventHandler<TradeDataSessionStatus> TradeStatusChanged;

        private string _account { get; set; }

        private ICosmosLogger cosmosLogger { get; set; }

        private HighGateProductInfo _vendorProductInfo { get; set; }

        private TradeDataSessionStatus _tradeStatus { get; set; }

        public TradeDataSessionStatus TradeStatus
        {
            get
            {
                return _tradeStatus;
            }
            set
            {
                lock (ActivePushJobLock)
                {
                    _tradeStatus = value;
                    if(_tradeStatus == TradeDataSessionStatus.Stopped)
                        ActivePushJobs.Clear();
                }
                _raiseStatusChanged();
            }
        }

        private void AddTimeOutString(string request_string)
        {
            //lock (ActiveRequestsLock)
            //{
           /* TimeOutRequestStrings.Enqueue($"{request_string} time:{DateTime.Now.ToLongTimeString()}");
            TimeOutRequestCount++;
            if (TimeOutRequestStrings.Count > 50)
            {
                TimeOutRequestStrings.Dequeue();
            }*/
            //}
        }

        // 如果服务端返回空包，则登记
        private void AddBadResponseString(string request_string, bool RecordCount = true)
        {
            //lock (ActiveRequestsLock)
            //{
            /*BadResponseRequesStrings.Enqueue($"{request_string} time:{DateTime.Now.ToLongTimeString()}");
            if (RecordCount)
            {
                BadResponseRequestCount++;
            }
            if (BadResponseRequesStrings.Count > 50)
            {
                BadResponseRequesStrings.Dequeue();
            }*/
            //}
        }

        public async Task<IUnSubscribeResult> StopPushJob(DataPushJob data_push_job, IUnSubscribeParameters unsubscriberParameter)
        {
            return await ChangeSubscription(data_push_job.Subscription, false, unsubscriberParameter);
        }

        private async Task<IUnSubscribeResult> ChangeSubscription(DataSubscription user_subscription, bool Isadd, IUnSubscribeParameters unsubscriberParameter)
        {
            var result = new UnSubscribeResult()
            {
                code = 0,
                msg = ""
            };

            bool removed = false;
            lock (ActivePushJobLock)
            {
                if (Isadd)
                {
                    AddActivePushJob(user_subscription.PushJob);
                }
                else
                {
                    removed = RemoveActivePushJob(user_subscription.PushJob);
                    

                }
            }

            if (removed)
            {
                var response = await ThirdRequest(unsubscriberParameter, true);
                result.code = response.code;
                result.msg = response.msg;
            }
          
            return result;
        }

        private void AddActiveRequest(DataRequest request)
        {
            ActiveRequests.TryAdd(request.uuid, request);
            //SendRequestTotalCount++;
        }
        private void RemoveActiveRequest(DataRequest request)
        {
            ActiveRequests.TryRemove(request.uuid, out var req);
        }

        private DataRequest FindActiveRequest(DataResponse response)
        {
            ActiveRequests.TryGetValue(response.uuid, out var sent_request);
            return sent_request;

        }

        private bool AddActivePushJob(DataPushJob push_job)
        {
            bool bSub = false;
            if (!ActivePushJobs.ContainsKey(push_job.Subscription.Topic))
            {
                bSub = true;
                ActivePushJobs.Add(push_job.Subscription.Topic, new List<DataPushJob>());
            }
            ActivePushJobs[push_job.Subscription.Topic].Add(push_job);
            return bSub;
        }
        private bool RemoveActivePushJob(DataPushJob push_job)
        {
            ActivePushJobs.TryGetValue(push_job.Subscription.Topic, out var push_jobs);
            push_jobs.Remove(push_job);
            if (push_jobs.Count == 0)
            {
                return true;
            }
            return false;
        }
        private List<DataPushJob> FindActivePushJobs(DataResponse response)
        {
            return FindActivePushJobs(response.Topic);
        }
        private List<DataPushJob> FindActivePushJobs(string shared_instance_key)
        {
            if (string.IsNullOrEmpty(shared_instance_key))
            {
                return null;
            }
            ActivePushJobs.TryGetValue(shared_instance_key, out var push_jobs);
            return push_jobs;
        }


        private ConcurrentDictionary<string, DataRequest> ActiveRequests { get; } = new ConcurrentDictionary<string, DataRequest>();

        private Dictionary<string, List<DataPushJob>> ActivePushJobs { get; } = new Dictionary<string, List<DataPushJob>>();

        private object ActivePushJobLock { get; } = new object();

    }

    public delegate void PushedEventHandler<DataResponseT>(DataPushJob pushJob, DataResponseT data_response);
}
