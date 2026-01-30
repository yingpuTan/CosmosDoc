using System;
using System.Collections.Generic;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1
{
    /// <summary>
    /// 会话状态
    /// </summary>
    public enum TradeDataSessionStatus
    {
        /// <summary>
        /// 已停止
        /// </summary>
        Stopped = 1,

        /// <summary>
        /// 启动中
        /// </summary>
        StartPending = 2,

        /// <summary>
        /// 停止中
        /// </summary>
        StopPending = 3,

        /// <summary>
        /// 运行中
        /// </summary>
        Running = 4,

        /// <summary>
        /// 唤醒中
        /// </summary>
        ContinuePending = 5,

        /// <summary>
        /// 暂停中
        /// </summary>
        PausePending = 6,

        /// <summary>
        /// 已暂停
        /// </summary>
        Paused = 7
    }

    /// <summary>
    /// 会话器
    /// </summary>
    public interface IDataSessionController
    {
        /// <summary>
        /// 启动会话
        /// </summary>
        /// <returns></returns>
        Task StartAsync(bool bMain = false);

        /// <summary>
        /// 停止会话
        /// </summary>
        /// <returns></returns>
        Task StopAsync();

        /// <summary>
        /// 设置账号和token信息
        /// </summary>
        /// <returns></returns>
        Task SetAccountToken(string account, string token, string password);

        /// <summary>
        /// 会话状态
        /// </summary>
        TradeDataSessionStatus TradeStatus { get; }

        /// <summary>
        /// 会话状态变更事件
        /// </summary>
        event EventHandler<TradeDataSessionStatus> TradeStatusChanged;
    }
}
