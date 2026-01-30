
using Cosmos.DataAccess.Trade.HighGate.v1;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.Internals.Protocol
{
    public class DataPushJob
    {
        public event PushedEventHandler<DataResponse> Pushed;

        public DataSubscription Subscription { get; set; }
        internal DataSessionController Session { get; set; }
        public bool IsBad { get; set; }

        /// <summary>
        /// 错误码 成功时返回0
        /// </summary>
        public int code { get; set; }

        /// <summary>
        /// 订阅结果
        /// </summary>
        public string msg { get; set; }
        public void RaisePushedEvent(DataResponse data_response)
        {
            //Logger.Info(Pushed.Target.ToString()); 
            Pushed?.Invoke(this, data_response);
        }
        public async Task<IUnSubscribeResult> Stop(IUnSubscribeParameters unsubscriberParameter)
        {
            Pushed = null;
            return await Session?.StopPushJob(this, unsubscriberParameter);
        }
    }
}
