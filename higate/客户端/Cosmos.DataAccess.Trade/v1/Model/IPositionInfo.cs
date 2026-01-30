using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 持仓信息
    /// </summary>
    public interface IPostionInfo : IAccountInfo, ISecurityInfo
    {
        /// <summary>
        /// 股票余额
        /// </summary>
        int StockBalance { get; set; }

        /// <summary>
        /// 可用数量
        /// </summary>
        int AvailableAmount { get; set; }

        /// <summary>
        /// 冻结数量
        /// </summary>
        int FrozenAmount { get; set; }

        /// <summary>
        /// 成本价
        /// </summary>
        double CostPrice { get; set; }

        /// <summary>
        /// 市价
        /// </summary>
        double MarketPrice { get; set; }

        /// <summary>
        /// 市值
        /// </summary>
        double MarketValue { get; set; }

        /// <summary>
        /// 盈亏
        /// </summary>
        double Profit { get; set; }

        /// <summary>
        /// 盈亏比例
        /// </summary>
        double ProfitRatio { get; set; }

        /// <summary>
        /// 盈亏比例
        /// </summary>
        double PositionRatio { get; set; }
    }
}
