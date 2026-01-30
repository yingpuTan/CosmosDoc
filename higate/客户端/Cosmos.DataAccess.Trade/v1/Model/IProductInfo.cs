using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
    
namespace Cosmos.DataAccess.Trade.v1.Model
{
    /// <summary>
    /// 本地机器信息
    /// </summary>
    public interface IProductInfo
    {
        /// <summary>
        /// 硬盘序列号MD5值
        /// </summary>
        string HDD { set; get; }

        /// <summary>
        /// MAC地址
        /// </summary>
        string MAC { set; get; }

        /// <summary>
        /// BIOS序列号
        /// </summary>
        string BIOS { set; get; }

        /// <summary>
        /// 硬盘序列号
        /// </summary>
        string Hddinfo { set; get; }

        /// <summary>
        /// 本机ip
        /// </summary>
        string IP { set; get; }

        /// <summary>
        /// CPU的ID
        /// </summary>
        string Cpuid { set; get; }

        /// <summary>
        /// CPU的信息
        /// </summary>
        string Cpuinfo { set; get; }

        /// <summary>
        /// BIOS日期
        /// </summary>
        string BiosDate { set; get; }

        /// <summary>
        /// 操作系统版本号
        /// </summary>
        string WinName { set; get; }

        /// <summary>
        /// 电脑名
        /// </summary>
        string ComputerName { set; get; }
    }
}
