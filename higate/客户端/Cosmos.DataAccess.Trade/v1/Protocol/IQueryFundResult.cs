using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 查询资金详情
    /// </summary>
    public interface IQueryFundResult : IAccountInfo
    {
        /// <summary>
        /// 资金余额
        /// </summary>
        double FundBalance { get; set; }

        /// <summary>
        /// 可用资金
        /// </summary>
        double AvailableFund { get; set; }

        /// <summary>
        /// 可用资金占比
        /// </summary>
        double AvailableFundRatio { get; set; }

        /// <summary>
        /// 冻结金额
        /// </summary>
        double FrozenFund { get; set; }

        /// <summary>
        /// 盈亏
        /// </summary>
        double Profit { get; set; }

        /// <summary>
        /// 股票市值
        /// </summary>
        double MarketValue { get; set; }

        /// <summary>
        /// 持仓占比
        /// </summary>
        double PositionRatio { get; set; }

        /// <summary>
        /// 总资产
        /// </summary>
        double TotalFund { get; set; }

    }
}
