using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    public interface IFundFlowInfo : IAccountInfo, ISecurityInfo
    {
        /// <summary>
        /// 操作
        /// </summary>
        OrderSide Side { get; set; }

        /// <summary>
        /// 成交数量
        /// </summary>
        int DealAmount { get; set; }

        /// <summary>
        /// 成交均价
        /// </summary>
        double AveragePrice { get; set; }

        /// <summary>
        /// 本次发生金额
        /// </summary>
        double ChangeMoney { get; set; }

        /// <summary>
        /// 本次金额
        /// </summary>
        double TotalMoney { get; set; }

        /// <summary>
        /// 手续费
        /// </summary>
        double Commission { get; set; }

        /// <summary>
        /// 印花税
        /// </summary>
        double Stamp { get; set; }

        /// <summary>
        /// 其他杂费
        /// </summary>
        double MiscFeeAmt { get; set; }
    }
}
