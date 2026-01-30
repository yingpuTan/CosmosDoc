using Cosmos.DataAccess.Trade.HighGate.v1.Model;
using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 银行流水信息
    /// </summary>
    public class BankFlowInfo : IBankFlowInfo
    {
        /// <summary>
        /// 发生日期
        /// </summary>
        public DateTime ChangeDate { get; set; }

        /// <summary>
        /// 发生金额
        /// </summary>
        public double ChangeMoney { get; set; }

        /// <summary>
        /// 银行名称
        /// </summary>
        public string BankName { get; set; }

        /// <summary>
        /// 详细内容
        /// </summary>
        public string Detail { get; set; }

        /// <summary>
        /// 流水ID
        /// </summary>
        public string FlowId { get; set; }

        /// <summary>
        /// 状态备注
        /// </summary>
        public string Note { get; set; }

        /// <summary>
        /// 操作
        /// </summary>
        public OrderType OrderType { get; set; }

        /// <summary>
        /// 状态
        /// </summary>
        public string OrderStatus { get; set; }

        /// <summary>
        /// 币种
        /// </summary>
        public Currency Currency { get; set; }
    }
}
