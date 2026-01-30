using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 委托撤单参数
    /// </summary>
    public interface ICancelOrderParameters : IAccountInfo
    {
        /// <summary>
        /// 委托编号
        /// </summary>
        string OrderId { get; set; }
    }
}
