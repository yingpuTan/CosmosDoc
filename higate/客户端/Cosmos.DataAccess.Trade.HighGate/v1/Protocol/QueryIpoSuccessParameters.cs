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
    /// 查询新股申购中签信息参数
    /// </summary>
    public class QueryIpoSuccessParameters : IQueryIpoSuccessParameters
    {
        /// <summary>
        /// 开始时间
        /// </summary>
        public DateTime BegineTime { get; set; }

        /// <summary>
        /// 结束时间
        /// </summary>
        public DateTime EndTime { get; set; }
        public string Account { get; set; }
        public string ShareholderAccount { get; set; }
        public string AccountName { get; set; }
        public AccountType AccountType { get; set; }
    }
}
