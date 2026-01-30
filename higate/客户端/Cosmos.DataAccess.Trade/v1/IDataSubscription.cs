using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1
{
    /// <summary>
    /// 订阅抽象
    /// </summary>
    public interface ITradeDataSubscription
    {
        /// <summary>
        /// 解订阅
        /// </summary>
        /// <returns></returns>
        Task<IUnSubscribeResult> UnsubscribeAsync(IUnSubscribeParameters unsubscriberParameter);

        /// <summary>
        /// 错误码 成功时返回0
        /// </summary>
        int code{ get; }

        /// <summary>
        /// 订阅结果
        /// </summary>
        string msg { get; }
    }
}
