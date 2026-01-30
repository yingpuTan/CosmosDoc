using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 中签详情
    /// </summary>
    public class IPOSuccessInfo : IIpoSuccessInfo
    {
        /// <summary>
        /// 中签时间
        /// </summary>
        public DateTime BusinessDate { get; set; }

        /// <summary>
        /// 中签数量
        /// </summary>
        public int OccurAmount { get; set; }

        /// <summary>
        /// 冻结数量
        /// </summary>
        public int FrozenAmount { get; set; }

        /// <summary>
        /// 股票成本价
        /// </summary>
        public double CostPrice { get; set; }

        /// <summary>
        /// 股票成交数量
        /// </summary>
        public int TradeAmount { get; set; }

        /// <summary>
        /// 委托数量
        /// </summary>
        public int OrderAmount { get; set; }

        /// <summary>
        /// 冻结金额
        /// </summary>
        public double FrozenMoney { get; set; }

        /// <summary>
        /// 操作（买卖方向）
        /// </summary>
        public OrderSide OrderSide { get; set; }

        /// <summary>
        /// 状态
        /// </summary>
        public string OrderStatus { get; set; }

        /// <summary>
        /// 备注
        /// </summary>
        public string Note { get; set; }
        public string SecurityID { get; set; }
        public string SecurityName { get; set; }
        public Market Market { get; set ; }
    }
}
