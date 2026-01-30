using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 持仓应答参数
    /// </summary>
    public interface IQueryPositionResult
    {
        /// <summary>
        /// 持仓数据
        /// </summary>
        IPostionInfo[] PostionInfos { get; set; }
    }
}
