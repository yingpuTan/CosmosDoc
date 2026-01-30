using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// etf申购盘后应答参数
    /// </summary>
    public interface IETFOrderOffTimeResult
    {
        /// <summary>
        /// 合同编号
        /// </summary>
        string OrderID { set; get; }
    }
}
