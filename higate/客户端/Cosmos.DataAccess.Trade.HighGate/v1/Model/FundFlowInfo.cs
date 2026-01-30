using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    public class FundFlowInfo : IFundFlowInfo
    {
        /// <summary>
        /// 操作
        /// </summary>
        public OrderSide Side { get; set; }

        /// <summary>
        /// 成交数量
        /// </summary>
        public int DealAmount { get; set; }

        /// <summary>
        /// 成交均价
        /// </summary>
        public double AveragePrice { get; set; }

        /// <summary>
        /// 本次发生金额
        /// </summary>
        public double ChangeMoney { get; set; }

        /// <summary>
        /// 本次金额
        /// </summary>
        public double TotalMoney { get; set; }

        /// <summary>
        /// 手续费
        /// </summary>
        public double Commission { get; set; }

        /// <summary>
        /// 印花税
        /// </summary>
        public double Stamp { get; set; }

        /// <summary>
        /// 其他杂费
        /// </summary>
        public  double MiscFeeAmt { get; set; }
        public string Account { get; set; }
        public string ShareholderAccount { get; set; }
        public string AccountName { get; set; }
        public AccountType AccountType { get; set; }
        public string SecurityID { get; set; }
        public string SecurityName { get; set; }
        public Market Market { get; set; }
    }
}
