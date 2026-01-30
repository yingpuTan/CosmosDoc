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
    /// 查询etf委托参数
    /// </summary>
    public class QueryETFOrderParameters : IQueryETFOrderParameters
    {
        /// <summary>
        /// 合同编号
        /// </summary>
        public string OrderID { get; set; }
        public string Account { get; set; }
        public string ShareholderAccount { get; set; }
        public string AccountName { get; set; }
        public AccountType AccountType { get; set; }
    }
}
