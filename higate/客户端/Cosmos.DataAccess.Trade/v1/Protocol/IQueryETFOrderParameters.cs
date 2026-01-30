using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{

    /// <summary>
    /// 查询etf委托参数
    /// </summary>
    public interface IQueryETFOrderParameters : IAccountInfo
    {
        /// <summary>
        /// 合同编号
        /// </summary>
        string OrderID { get; set; }
    }
}
