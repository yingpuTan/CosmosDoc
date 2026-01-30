using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 查询配售额度应答参数
    /// </summary>
    public interface IQueryIpoSharesResult
    {
        /// <summary>
        /// 额度信息
        /// </summary>
        IIPOSharesInfo[] iPOSharesInfos { get; set; }
    }
}
