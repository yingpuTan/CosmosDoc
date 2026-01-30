using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 证券代码信息
    /// </summary>
    public interface ISecurityInfo
    {
        /// <summary>
        /// 证券代码
        /// </summary>
        string SecurityID { get; set; }

        /// <summary>
        /// 证券名称
        /// </summary>
        string SecurityName { get; set; }

        /// <summary>
        /// 市场
        /// </summary>
        Market Market { get; set; }
    }
}
