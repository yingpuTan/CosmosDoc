using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 中签详情
    /// </summary>
    public interface IIpoSuccessInfo : ISecurityInfo
    {
        /// <summary>
        /// 中签时间
        /// </summary>
        DateTime BusinessDate { get; set; }

        /// <summary>
        /// 中签数量
        /// </summary>
        int OccurAmount { get; set; }

        /// <summary>
        /// 冻结数量
        /// </summary>
        int FrozenAmount { get; set; }

        /// <summary>
        /// 股票成本价
        /// </summary>
        double CostPrice { get; set; }

        /// <summary>
        /// 股票成交数量
        /// </summary>
        int TradeAmount { get; set; }

        /// <summary>
        /// 委托数量
        /// </summary>
        int OrderAmount { get; set; }

        /// <summary>
        /// 冻结金额
        /// </summary>
        double FrozenMoney { get; set; }

        /// <summary>
        /// 操作（买卖方向）
        /// </summary>
        OrderSide OrderSide { get; set; }

        /// <summary>
        /// 状态
        /// </summary>
        string OrderStatus { get; set; }

        /// <summary>
        /// 备注
        /// </summary>
        string Note { get; set; }
    }
}
