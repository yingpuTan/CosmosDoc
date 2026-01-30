using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /**
     * 表示订阅结果的接口。
     * 该接口定义了订阅操作的结果，包括订阅的主题和响应消息列表。
     */
    public interface ISubscribeResult {
        /**
         * 获取或设置订阅的主题。
         * 主题是订阅操作的目标，用于指定需要接收的消息类型。
         */
        String topic { get; set; }
    
        /**
         * 获取或设置响应消息列表。
         * 该列表包含了订阅操作的响应消息，每个消息都实现了ISubscribeReqMsg接口。
         */
        IList<ISubscribeReqMsg> rsp { get; set; }
    }


    public interface IUnSubscribeResult
    {

        int code { get; set; }

        string msg { get; set; }
    }
}
