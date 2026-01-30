using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json.Nodes;
using System.Threading.Tasks;
using Cosmos.DataAccess.Trade.v1.Protocol;

namespace Cosmos.DataAccess.Trade.HighGate.Internals.Protocol
{
    /// <summary>
    /// 应答类定义
    /// </summary>
    public class DataResponse: IResponseTime
    {
        /**
       * 唯一标识符（UID），用于唯一标识每个请求。
       */
        public string uuid { get; set; }

        /**
         * 参数表示传递给服务方法的参数。
         */
        public string response { get; set; }

        public bool IsBad { get; set; } = false;

        public string Topic { get; set; }

        // 发送时间节点
        public DateTime sendtime { get; set; }

        // 接受时间节点
        public DateTime recvtime { get; set; }

        public bool IsTimeOutResponse { get; set; } = false;// 是否是延迟包，延迟包或者没回包

    }

    //解析错误返回
    public class ErrorResponse
    {
        public int code { get; set; }
        public string message { get; set; }
    }
}
