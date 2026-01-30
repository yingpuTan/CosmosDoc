using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 登录参数
    /// </summary>
    public interface IAccountLoginParameters : IAccountInfo
    {
        /// <summary>
        /// 密码
        /// </summary>
        string Password { set; get; }

        /// <summary>
        /// 通讯密码
        /// </summary>
        string TxPassword { set; get; }

        /// <summary>
        /// 本地机器信息
        /// </summary>
        IProductInfo ProductInfo { set; get; }
    }
}
