using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 查询成交应答参数
    /// </summary>
    public interface IQueryDealResult
    {
        /// <summary>
        /// 成交信息
        /// </summary>
        IDealInfo[] DealInfos { get; set; }
    }
}
