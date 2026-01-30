using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 新股配售额度信息
    /// </summary>
    public class IPOSharesInfo : IIPOSharesInfo
    {
        /// <summary>
        /// 市场名称
        /// </summary>
        public Market Market { get; set; }

        /// <summary>
        /// AvailableStockBalance
        /// </summary>
        public int AvailableStockBalance { get; set; }
        public string Account {  get; set; }
        public string ShareholderAccount {  get; set; }
        public string AccountName {  get; set; }
        public AccountType AccountType {  get; set; }
    }
}
