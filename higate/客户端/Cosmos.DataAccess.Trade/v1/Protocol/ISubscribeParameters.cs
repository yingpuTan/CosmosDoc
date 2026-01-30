using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /// <summary>
    /// 定义推送配置接口。
    /// </summary>
    public interface IPushConfig
    {
        /// <summary>
        /// 获取或设置推送间隔（以秒为单位）。
        /// </summary>
        int push_interval { get; set; }
    }
    
    /// <summary>
    /// 定义订阅请求消息接口。
    /// </summary>
    public interface ISubscribeReqMsg
    {
        /// <summary>
        /// 获取或设置请求消息的参数字典。
        /// </summary>
        IDictionary<string, string> param { get; set; }
    
        /// <summary>
        /// 获取或设置推送配置。
        /// </summary>
        IPushConfig push_config { get; set; }
    }
    
    /// <summary>
    /// 定义订阅参数。
    /// </summary>
    public interface ISubscribeParameters : IThirdRequestParameters
    {
       
    }

    /// <summary>
    /// 定义取消订阅参数。
    /// </summary>
    public interface IUnSubscribeParameters : IThirdRequestParameters
    {

    }
}
