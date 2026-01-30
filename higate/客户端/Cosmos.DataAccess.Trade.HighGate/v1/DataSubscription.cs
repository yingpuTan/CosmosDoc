using Cosmos.DataAccess.Trade.HighGate.Internals.Protocol;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1
{
    public sealed class TradeDataSubscription : ITradeDataSubscription
    {
        public TradeDataSubscription(IEnumerable<DataPushJob> dataPushJobs)
        {
            _dataPushJobs = dataPushJobs.ToArray();
        }
        public async Task<IUnSubscribeResult> UnsubscribeAsync(IUnSubscribeParameters unsubscriberParameter)
        {
            IUnSubscribeResult result = null;
            foreach (var job in _dataPushJobs)
            {
                result = await job.Stop(unsubscriberParameter);
            }
            return result;
        }
        public IReadOnlyCollection<DataPushJob> _dataPushJobs { get; }

        /// <summary>
        /// 错误码 成功时返回0
        /// </summary>
        public int code { get; set; }

        /// <summary>
        /// 订阅结果
        /// </summary>
        public string msg { get; set; }
    }

}
