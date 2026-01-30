using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 委托类型
    /// </summary>
    public enum OrderType
    {
        /// <summary>
        /// 限价
        /// </summary>
        Limit,

        /// <summary>
        /// 市价策略
        /// </summary>
        MarktStrategy,
    }
}
