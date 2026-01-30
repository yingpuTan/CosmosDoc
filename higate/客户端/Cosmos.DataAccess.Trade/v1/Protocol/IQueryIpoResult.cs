using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 新股申购应答参数
    /// </summary>
    public interface IQueryIpoResult
    {
        /// <summary>
        /// 新股信息
        /// </summary>
        IIPOInfo[] IposInfos { get; set; }
    }

}
