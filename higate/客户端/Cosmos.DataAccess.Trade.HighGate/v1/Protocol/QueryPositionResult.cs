using Cosmos.DataAccess.Trade.HighGate.v1.Model;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    /// <summary>
    /// 持仓应答参数
    /// </summary>
    public class QueryPositionResult : IQueryPositionResult
    {
        /// <summary>
        /// 持仓数据
        /// </summary>
        public IPostionInfo[] PostionInfos { get; set; }
    }
}
