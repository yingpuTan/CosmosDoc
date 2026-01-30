using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 银行流水信息
    /// </summary>
    public interface IBankFlowInfo
    {
        /// <summary>
        /// 发生日期
        /// </summary>
        DateTime ChangeDate { get; set; }

        /// <summary>
        /// 发生金额
        /// </summary>
        double ChangeMoney { get; set; }

        /// <summary>
        /// 银行名称
        /// </summary>
        string BankName { get; set; }

        /// <summary>
        /// 详细内容
        /// </summary>
        string Detail { get; set; }

        /// <summary>
        /// 流水ID
        /// </summary>
        string FlowId { get; set; }

        /// <summary>
        /// 状态备注
        /// </summary>
        string Note { get; set; }

        /// <summary>
        /// 操作
        /// </summary>
        OrderType OrderType { get; set; }

        /// <summary>
        /// 状态
        /// </summary>
        string OrderStatus { get; set; }

        /// <summary>
        /// 币种
        /// </summary>
        Currency Currency { get; set; }
    }
}
