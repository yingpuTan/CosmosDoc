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
    /// 查询持仓参数
    /// </summary>
    public class QueryPositionParameters : IQueryPositionParameters
    {
        public string Account { get; set;}
        public string ShareholderAccount { get; set;}
        public string AccountName { get; set;}
        public AccountType AccountType { get; set;}
        public string SecurityID { get; set;}
        public string SecurityName { get; set;}
        public Market Market { get; set;}
    }
}
