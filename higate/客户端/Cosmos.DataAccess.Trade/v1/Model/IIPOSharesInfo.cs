using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 新股配售额度信息
    /// </summary>
    public interface IIPOSharesInfo : IAccountInfo
    {
        /// <summary>
        /// 市场名称
        /// </summary>
        Market Market { get; set; }

        /// <summary>
        /// AvailableStockBalance
        /// </summary>
        int AvailableStockBalance { get; set; }
    }
}
