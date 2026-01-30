using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 查询资金流水应答参数
    /// </summary>
    public interface IQueryFundsFlowResult
    {
        /// <summary>
        /// 流水明细
        /// </summary>
        IFundFlowInfo[] FundFlowInfos { get; set; }
    }
}
