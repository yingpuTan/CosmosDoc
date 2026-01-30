using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    public interface IAccountInfo
    {
        /// <summary>
        /// 资金账号
        /// </summary>
        string Account { get; set; }

        /// <summary>
        /// 股东帐号
        /// </summary>
        string ShareholderAccount { get; set; }

        /// <summary>
        /// 账户名称
        /// </summary>
        string AccountName { get; set; }

        /// <summary>
        /// 账户类型
        /// </summary>
        AccountType AccountType { get; set; }
    }
}
