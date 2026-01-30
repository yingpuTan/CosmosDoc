using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 新股信息
    /// </summary>
    public interface IIPOInfo : ISecurityInfo
    {
        /// <summary>
        /// 可申购数量
        /// </summary>
        string AvailablePurchaseAmt { get; set; }

        /// <summary>
        /// 配售额度
        /// </summary>
        string AvailableStockBalance { get; set; }

        /// <summary>
        /// 时间
        /// </summary>
        DateTime DealDate { get; set; }

        /// <summary>
        /// 最大可买量
        /// </summary>
        int MaxPurchaseAmount { get; set; }

        /// <summary>
        /// 最小可买量
        /// </summary>
        int MinPurchaseAmt { get; set; }

        /// <summary>
        /// 委托数量
        /// </summary>
        int OrderQty { get; set; }

        /// <summary>
        /// PurchasePrice
        /// </summary>
        double PurchasePrice { get; set; }
    }
}
