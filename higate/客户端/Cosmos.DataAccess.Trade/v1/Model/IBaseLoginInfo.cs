using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    public interface IBaseLoginInfo
    {
        /// <summary>
        /// 产品名
        /// </summary>
        string ProductID { get; }

        /// <summary>
        /// 账户名
        /// </summary>
        string Account { get; }

        /// <summary>
        /// 密码
        /// </summary>
        string PassWord { get; }

        /// <summary>
        /// 本次登录token
        /// </summary>
        string Token { get; }

        /// <summary>
        /// 消息中心ip
        /// </summary>
        string Ip { get; }

        /// <summary>
        /// 消息中心端口
        /// </summary>
        int Port { get;}
    }
}
