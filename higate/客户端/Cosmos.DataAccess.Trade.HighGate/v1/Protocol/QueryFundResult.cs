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
    /// 查询资金详情
    /// </summary>
    public class QueryFundResult : IQueryFundResult
    {
        /// <summary>
        /// 资金余额
        /// </summary>
        public double FundBalance { get; set; }

        /// <summary>
        /// 可用资金
        /// </summary>
        public double AvailableFund { get; set; }

        /// <summary>
        /// 可用资金占比
        /// </summary>
        public double AvailableFundRatio { get; set; }

        /// <summary>
        /// 冻结金额
        /// </summary>
        public double FrozenFund { get; set; }

        /// <summary>
        /// 盈亏
        /// </summary>
        public double Profit { get; set; }

        /// <summary>
        /// 股票市值
        /// </summary>
        public double MarketValue { get; set; }

        /// <summary>
        /// 持仓占比
        /// </summary>
        public double PositionRatio { get; set; }

        /// <summary>
        /// 总资产
        /// </summary>
        public double TotalFund { get; set; }
        public string Account {get; set; }
        public string ShareholderAccount {get; set; }
        public string AccountName {get; set; }
        public AccountType AccountType {get; set; }
    }
}
