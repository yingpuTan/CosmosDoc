using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 应用推荐接口。
    /// 该接口定义了应用推荐的基本属性，包括应用的唯一标识、应用名称和推荐内容。
    /// </summary>
    public interface IAppRecommand
    {
        /// <summary>
        /// 获取应用的唯一标识（GUID）。
        /// </summary>
        string AppGuid { get; }

        /// <summary>
        /// 获取应用的名称。
        /// </summary>
        string AppName { get; }

        /// <summary>
        /// 获取推荐内容。
        /// </summary>
        string Content { get; }
    }

    /// <summary>
    /// 报警提示接口。
    /// </summary>
    public interface IWarningTip
    {
        /// <summary>
        /// 报警内容。
        /// </summary>
        string Content { get; }
    }
}