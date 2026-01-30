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
    /// 登录参数
    /// </summary>
    public class AccountLoginParameters : IAccountLoginParameters
    {
        /// <summary>
        /// 密码
        /// </summary>
        public string Password { set; get; }

        /// <summary>
        /// 通讯密码
        /// </summary>
        public string TxPassword { set; get; }

        /// <summary>
        /// 本地机器信息
        /// </summary>
        public string Account { get; set; }

        public string ShareholderAccount { get; set; }

        public string AccountName { get; set; }

        public IProductInfo ProductInfo { get; set; }

        public AccountType AccountType { get ; set; }
    }
}
