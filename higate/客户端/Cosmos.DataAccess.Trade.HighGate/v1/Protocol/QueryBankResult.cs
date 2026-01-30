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
    /// 查询银行参数接口
    /// </summary>
    public class QueryBankResult: IQueryBankResult
    {
        /// <summary>
        /// 银行信息
        /// </summary>
        public IBankInfo BankInfos { get; set; }
    }
}
