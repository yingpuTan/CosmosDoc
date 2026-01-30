using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 成交信息
    /// </summary>
    public interface IDealInfo : IOrderInfo
    {
        /// <summary>
        /// 成交时间
        /// </summary>
        DateTime DealTime { set; get; }

        /// <summary>
        /// 成交编号
        /// </summary>
        string ExecID { set; get; }
    }
}
