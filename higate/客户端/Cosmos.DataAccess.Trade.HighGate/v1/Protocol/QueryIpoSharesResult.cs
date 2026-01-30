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
    /// 查询配售额度应答参数
    /// </summary>
    public class QueryIpoSharesResult : IQueryIpoSharesResult
    {
        /// <summary>
        /// 额度信息
        /// </summary>
        public IIPOSharesInfo[] iPOSharesInfos { get; set; }
    }
}
