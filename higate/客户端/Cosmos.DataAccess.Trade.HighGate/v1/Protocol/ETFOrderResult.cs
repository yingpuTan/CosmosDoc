using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    /// <summary>
    /// etf申购应答参数
    /// </summary>
    public class ETFOrderResult: IETFOrderResult
    {
        /// <summary>
        /// 合同编号
        /// </summary>
        public string OrderID {  get; set; }
    }
}
