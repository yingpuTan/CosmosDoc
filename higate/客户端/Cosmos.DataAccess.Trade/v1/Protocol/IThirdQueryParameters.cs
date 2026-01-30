using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /**
     * 定义第三方查询参数的接口。
     * 该接口包含三个属性，用于描述第三方服务的查询请求。
     */
    public interface IThirdRequestParameters
    {
        /**
          * 获取或设置id。
          * 请求唯一标识
        */
        string uuid { get; set; }

        /**
         * 获取或设置操作名称。
         * 例如，"GetUserData" 或 "UpdateUser"。
         */
        string Action { get; set; }
    
        /**
         * 获取或设置查询参数。
         * 序列化串。
         */
        string Parameters { get; set; }

        /**
         * 获取或设置路由信息。
         * 用于指定请求的目标路由。
         * 为空默认发给upb_cosmos_plugin_req
         */
        string Server { get; set; }

        /// <summary>
        /// 订阅的topic
        /// 请求时该参数不填写，订阅时该字段必须填写
        /// </summary>
        string Topic { get; set; }
    }
}
