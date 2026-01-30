using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// etf盘后赎回参数
    /// </summary>
    public interface IETFRedeemOffTimeParameters : IAccountInfo, ISecurityInfo
    {
        /// <summary>
        /// 赎回数量
        /// </summary>
        int OrderAmount { get; set; }
    }
}
