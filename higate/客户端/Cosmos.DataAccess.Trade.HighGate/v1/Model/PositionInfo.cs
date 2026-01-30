using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 持仓信息
    /// </summary>
    public class PostionInfo : IPostionInfo
    {
        /// <summary>
        /// 股票余额
        /// </summary>
        public int StockBalance { get; set; }

        /// <summary>
        /// 可用数量
        /// </summary>
        public int AvailableAmount { get; set; }

        /// <summary>
        /// 冻结数量
        /// </summary>
        public int FrozenAmount { get; set; }

        /// <summary>
        /// 成本价
        /// </summary>
        public double CostPrice { get; set; }

        /// <summary>
        /// 市价
        /// </summary>
        public double MarketPrice { get; set; }

        /// <summary>
        /// 市值
        /// </summary>
        public double MarketValue { get; set; }

        /// <summary>
        /// 盈亏
        /// </summary>
        public double Profit { get; set; }

        /// <summary>
        /// 盈亏比例
        /// </summary>
        public double ProfitRatio { get; set; }

        /// <summary>
        /// 盈亏比例
        /// </summary>
        public double PositionRatio { get; set; }
        public string Account { get; set; }
        public string ShareholderAccount { get; set; }
        public string AccountName { get; set; }
        public AccountType AccountType { get; set; }
        public string SecurityID { get; set; }
        public string SecurityName { get; set; }
        public Market Market { get; set; }
    }
}
