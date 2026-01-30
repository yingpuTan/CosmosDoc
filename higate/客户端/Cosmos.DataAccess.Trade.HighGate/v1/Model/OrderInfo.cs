using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 单挑委托信息
    /// </summary>
    public class OrderInfo : IOrderInfo
    {
        /// <summary>
        /// 成交均价
        /// </summary>
        public double AveragePrice { set; get; }

        /// <summary>
        /// 委托数量
        /// </summary>
        public int OrderAmount { set; get; }

        /// <summary>
        /// 委托编号
        /// </summary>
        public string OrderID { set; get; }

        /// <summary>
        /// 委托状态
        /// </summary>
        public int OrderStatus { set; get; }

        /// <summary>
        /// 成交数量
        /// </summary>
        public int DealAmount { set; get; }

        /// <summary>
        /// 委托时间
        /// </summary>
        public DateTime OrderTime { set; get; }

        /// <summary>
        /// 委托方向
        /// </summary>
        public OrderSide Side { set; get; }

        /// <summary>
        /// 委托类型
        /// </summary>
        public OrderType type { set; get; }
        public string SecurityID { set; get; }
        public string SecurityName { set; get; }
        public Market Market { set; get; }
        public string Account { set; get; }
        public string ShareholderAccount { set; get; }
        public string AccountName { set; get; }
        public AccountType AccountType { set; get; }
    }
}
