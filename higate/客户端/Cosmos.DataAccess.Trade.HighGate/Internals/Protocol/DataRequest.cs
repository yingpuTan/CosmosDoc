using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Threading;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.Internals.Protocol
{
    public class RequestStatus
    {
        // 对应的服务端的回包
        public DataResponse Response { get; set; }

        // 发送前时间节点
        public DateTime BeforeSendDateTime { get; set; }

        // 发送后时间节点
        public DateTime AfterSendDateTime { get; set; }
    }

    /// <summary>
    /// 请求类定义
    /// </summary>
    public class DataRequest
    {
        /**
       * 唯一标识符（UID），用于唯一标识每个请求。
       */
        public string uuid { get; set; }

        /**
         * 服务名称，表示请求的目标服务。
         */
        public string servicename { get; set; }

        /**
         * 方法名称，表示请求调用的服务方法。
         */
        public string method { get; set; }

        /**
         * 参数表示传递给服务方法的参数。
         */
        public JObject parameters { get; set; }

        /**
        * 请求类型
        */
        public string datatype { get; set; } = "json";

        public string token { get; set; }

        public ManualResetEventSlim ResponsedEvent { get; } = new ManualResetEventSlim();

        public RequestStatus Status { get; set; } = new RequestStatus();
        public byte[] Serialize()
        {
            JObject context = new JObject();
            JObject request = new JObject();
            context["id"] = uuid;
            context["method"] = method;
            context["servicename"] = servicename;
            context["params"] = parameters;
            context["token"] = token;
            request["datatype"] = datatype;

            if (datatype == "json")
            {
                request["content"] = context.ToString(Newtonsoft.Json.Formatting.None);
            }
            string strSend = request.ToString(Newtonsoft.Json.Formatting.None);
            return Encoding.GetEncoding("GBK").GetBytes(strSend);
        }
    }
}
