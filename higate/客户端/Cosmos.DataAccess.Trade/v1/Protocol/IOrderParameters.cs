using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 委托参数
    /// </summary>
    public interface IOrderParameters : IAccountInfo, ISecurityInfo
    {
        /// <summary>
        /// 委托价格
        /// </summary>
        double OrderPrice { set; get; }

        /// <summary>
        /// 委托数量
        /// </summary>
        int OrderAmount { set; get; }

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
