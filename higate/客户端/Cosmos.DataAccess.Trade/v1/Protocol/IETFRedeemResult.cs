using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// etf赎回应答参数
    /// </summary>
    public interface IETFRedeemResult
    {
        /// <summary>
        /// 委托编号
        /// </summary>
        string OrderID { get; set; }
    }
}
