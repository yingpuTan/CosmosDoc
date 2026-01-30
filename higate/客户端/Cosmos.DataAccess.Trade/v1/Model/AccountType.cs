using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 账户类型
    /// </summary>
    public enum AccountType
    {
        /// <summary>
        /// 证券账户
        /// </summary>
        Security,

        /// <summary>
        /// 信用
        /// </summary>
        Credit,

        /// <summary>
        /// 期货账户
        /// </summary>
        Future,
    }
}
