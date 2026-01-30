using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 新股信息
    /// </summary>
    public class IPOInfo : IIPOInfo
    {
        /// <summary>
        /// 可申购数量
        /// </summary>
        public string AvailablePurchaseAmt { get; set; }

        /// <summary>
        /// 配售额度
        /// </summary>
        public string AvailableStockBalance { get; set; }

        /// <summary>
        /// 时间
        /// </summary>
        public DateTime DealDate { get; set; }

        /// <summary>
        /// 最大可买量
        /// </summary>
        public int MaxPurchaseAmount { get; set; }

        /// <summary>
        /// 最小可买量
        /// </summary>
        public int MinPurchaseAmt { get; set; }

        /// <summary>
        /// 委托数量
        /// </summary>
        public int OrderQty { get; set; }

        /// <summary>
        /// PurchasePrice
        /// </summary>
        public double PurchasePrice { get; set; }
        public string SecurityID { get; set; }
        public string SecurityName { get; set; }
        public Market Market { get; set; }
    }
}
