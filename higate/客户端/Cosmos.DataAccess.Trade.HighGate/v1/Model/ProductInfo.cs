using Cosmos.DataAccess.Trade.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
    
namespace Cosmos.DataAccess.Trade.HighGate.v1.Model
{
    /// <summary>
    /// 本地机器信息
    /// </summary>
    public class ProductInfo : IProductInfo
    {
        /// <summary>
        /// 硬盘序列号MD5值
        /// </summary>
        public string HDD { set; get; }

        /// <summary>
        /// MAC地址
        /// </summary>
        public string MAC { set; get; }

        /// <summary>
        /// BIOS序列号
        /// </summary>
        public string BIOS { set; get; }

        /// <summary>
        /// 硬盘序列号
        /// </summary>
        public string Hddinfo { set; get; }

        /// <summary>
        /// 本机ip
        /// </summary>
        public string IP { set; get; }

        /// <summary>
        /// CPU的ID
        /// </summary>
        public string Cpuid { set; get; }

        /// <summary>
        /// CPU的信息
        /// </summary>
        public string Cpuinfo { set; get; }

        /// <summary>
        /// BIOS日期
        /// </summary>
        public string BiosDate { set; get; }

        /// <summary>
        /// 操作系统版本号
        /// </summary>
        public string WinName { set; get; }

        /// <summary>
        /// 电脑名
        /// </summary>
        public string ComputerName { set; get; }
    }
}
