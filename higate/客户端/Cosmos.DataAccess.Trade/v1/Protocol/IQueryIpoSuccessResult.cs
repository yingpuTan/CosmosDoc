using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 中签明细应答参数
    /// </summary>
    public interface IQueryIpoSuccessResult
    {
        /// <summary>
        /// 中签明细
        /// </summary>
        IIpoSuccessInfo[] ipoSuccessInfos { get; set; }
    }
}
