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
    /// 中签明细应答参数
    /// </summary>
    public class QueryIpoSuccessResult : IQueryIpoSuccessResult
    {
        /// <summary>
        /// 中签明细
        /// </summary>
        public IIpoSuccessInfo[] ipoSuccessInfos { get; set; }
    }
}
