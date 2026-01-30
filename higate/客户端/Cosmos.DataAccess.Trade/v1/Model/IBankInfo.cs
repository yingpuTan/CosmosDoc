using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 银行信息
    /// </summary>
    public interface IBankInfo
    {
        /// <summary>
        /// 银行代码
        /// </summary>
        string BankCode { get; set; }

        /// <summary>
        /// 银河名称
        /// </summary>
        string BankName { get; set; }

        /// <summary>
        /// 主题
        /// </summary>
        string BankNeedPwd { get; set; }

        /// <summary>
        /// 银行账号
        /// </summary>
        string BankAccount { get; set; }

        /// <summary>
        /// 查询银行选项
        /// </summary>
        string BankOption { get; set; }

        /// <summary>
        /// 币种
        /// </summary>
        Currency Currency { get; set; }
    }
}
