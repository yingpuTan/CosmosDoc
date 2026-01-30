using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 查询资金流水参数
    /// </summary>
    public interface IQueryFundsFlowParameters : IAccountInfo
    {
        /// <summary>
        /// 开始时间
        /// </summary>
        DateTime BegineTime { get; set; }

        /// <summary>
        /// 结束时间
        /// </summary>
        DateTime EndTime { get; set; }
    }
}
