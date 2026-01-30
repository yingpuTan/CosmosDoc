using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    /// <summary>
    /// 委托应答
    /// </summary>
    public class OrderResult: IOrderResult
    {
        /// <summary>
        /// 合同编号
        /// </summary>
        public string OrderId { get; set; }
    }
}
