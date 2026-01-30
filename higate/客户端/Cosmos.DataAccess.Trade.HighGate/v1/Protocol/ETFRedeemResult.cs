using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    /// <summary>
    /// etf赎回应答参数
    /// </summary>
    public class ETFRedeemResult : IETFRedeemResult
    {
        /// <summary>
        /// 委托编号
        /// </summary>
        public string OrderID { get; set; }
    }
}
