using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 成交信息
    /// </summary>
    public class DealInfo : IDealInfo
    {
        /// <summary>
        /// 成交时间
        /// </summary>
        public DateTime DealTime { set; get; }

        /// <summary>
        /// 成交编号
        /// </summary>
        public string ExecID { set; get; }
        public double AveragePrice { set; get; }
        public int OrderAmount {  set; get; }
        public string OrderID {  set; get; }
        public int OrderStatus {  set; get; }
        public int DealAmount {  set; get; }
        public DateTime OrderTime {  set; get; }
        public OrderSide Side {  set; get; }
        public OrderType type {  set; get; }
        public string SecurityID {  set; get; }
        public string SecurityName {  set; get; }
        public Market Market {  set; get; }
        public string Account {  set; get; }
        public string ShareholderAccount {  set; get; }
        public string AccountName {  set; get; }
        public AccountType AccountType {  set; get; }
    }
}
