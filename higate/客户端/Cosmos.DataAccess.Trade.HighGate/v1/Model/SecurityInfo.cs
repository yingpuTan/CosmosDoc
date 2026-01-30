using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 证券代码信息
    /// </summary>
    public class SecurityInfo: ISecurityInfo
    {
        /// <summary>
        /// 证券代码
        /// </summary>
        public string SecurityID { get; set; }

        /// <summary>
        /// 证券名称
        /// </summary>
        public string SecurityName { get; set; }

        /// <summary>
        /// 市场
        /// </summary>
        public Market Market { get; set; }
    }
}
