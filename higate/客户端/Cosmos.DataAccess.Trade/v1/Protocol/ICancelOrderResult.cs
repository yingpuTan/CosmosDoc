using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 委托撤单应答参数
    /// </summary>
    public interface ICancelOrderResult
    {
        /// <summary>
        /// 合同编号
        /// </summary>
        string OrderId { get; set; }
    }
}
