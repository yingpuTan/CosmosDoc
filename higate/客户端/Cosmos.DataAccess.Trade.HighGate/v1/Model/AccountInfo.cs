using Cosmos.DataAccess.Trade.HighGate.v1.Model;
using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    public class AccountInfo : IAccountInfo
    {
        /// <summary>
        /// 资金账号
        /// </summary>
        public string Account { get; set; }

        /// <summary>
        /// 股东帐号
        /// </summary>
        public string ShareholderAccount { get; set; }

        /// <summary>
        /// 账户名称
        /// </summary>
        public string AccountName { get; set; }

        /// <summary>
        /// 账户类型
        /// </summary>
        public AccountType AccountType { get; set; }
    }
}
