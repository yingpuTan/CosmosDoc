using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    public class ThirdQueryParameters : IThirdRequestParameters
    {
        /**
          * 获取或设置id。
          * 请求唯一标识
        */
        public string uuid { get; set; }

        /**
         * 获取或设置操作名称。
         * 例如，"GetUserData" 或 "UpdateUser"。
         */
        public string Action { get; set; }

        /**
         * 获取或设置查询参数。
         * 序列化串。
         */
        public string Parameters { get; set; }

        /**
         * 获取或设置路由信息。
         * 用于指定请求的目标路由。
         * 为空默认发给upb_cosmos_plugin_req
         */
        public string Server { get; set; }

        /// <summary>
        /// 订阅的topic
        /// </summary>
        public string Topic { get; set; } = "";
    }
}
