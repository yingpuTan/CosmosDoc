using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 单挑委托信息
    /// </summary>
    public interface IOrderInfo : ISecurityInfo, IAccountInfo
    {
        /// <summary>
        /// 成交均价
        /// </summary>
        double AveragePrice { set; get; }

        /// <summary>
        /// 委托数量
        /// </summary>
        int OrderAmount { set; get; }

        /// <summary>
        /// 委托编号
        /// </summary>
        string OrderID { set; get; }

        /// <summary>
        /// 委托状态
        /// </summary>
        int OrderStatus { set; get; }

        /// <summary>
        /// 成交数量
        /// </summary>
        int DealAmount { set; get; }

        /// <summary>
        /// 委托时间
        /// </summary>
        DateTime OrderTime { set; get; }

        /// <summary>
        /// 委托方向
        /// </summary>
        OrderSide Side { set; get; }

        /// <summary>
        /// 委托类型
        /// </summary>
        OrderType type { set; get; }
    }
}
