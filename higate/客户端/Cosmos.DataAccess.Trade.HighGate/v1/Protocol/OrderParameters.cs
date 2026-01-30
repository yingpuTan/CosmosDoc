using Cosmos.DataAccess.Trade.HighGate.v1.Model;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    /// <summary>
    /// 委托参数
    /// </summary>
    public class OrderParameters : IOrderParameters
    {
        public string Account { get; set; }
        public string ShareholderAccount { get; set; }
        public string AccountName { get; set; }
        public AccountType AccountType { get; set; }
        public string SecurityID { get; set; }
        public string SecurityName { get; set; }
        public Market Market { get; set; }

        /// <summary>
        /// 委托价格
        /// </summary>
        double OrderPrice { set; get; }
        double IOrderParameters.OrderPrice { get; set; }

        /// <summary>
        /// 委托数量
        /// </summary>
        int OrderAmount { set; get; }
        int IOrderParameters.OrderAmount { get; set; }

        /// <summary>
        /// 委托方向
        /// </summary>
        OrderSide Side { set; get; }
        OrderSide IOrderParameters.Side { get; set; }


        /// <summary>
        /// 委托类型
        /// </summary>
        OrderType type { set; get; }
        OrderType IOrderParameters.type { get; set; }
    }
}
