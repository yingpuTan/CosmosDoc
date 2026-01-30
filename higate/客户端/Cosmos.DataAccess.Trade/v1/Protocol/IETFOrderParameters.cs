using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// ETF申购参数
    /// </summary>
    public interface IETFOrderParameters : IAccountInfo, ISecurityInfo
    {
        /// <summary>
        /// 申购数量
        /// </summary>
        int OrderAmount { get; set; }
    }
}
