using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 银行信息
    /// </summary>
    public class BankInfo : IBankInfo
    {
        /// <summary>
        /// 银行代码
        /// </summary>
        public string BankCode { get; set; }

        /// <summary>
        /// 银河名称
        /// </summary>
        public string BankName { get; set; }

        /// <summary>
        /// 主题
        /// </summary>
        public string BankNeedPwd { get; set; }

        /// <summary>
        /// 银行账号
        /// </summary>
        public string BankAccount { get; set; }

        /// <summary>
        /// 查询银行选项
        /// </summary>
        public string BankOption { get; set; }

        /// <summary>
        /// 币种
        /// </summary>
        public Currency Currency { get; set; }
    }
}
