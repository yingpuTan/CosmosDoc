using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    /**
     * 定义一个第三方查询结果的接口。
     * 该接口包含三个属性，分别表示查询结果的数据、状态码和消息。
     */
    public interface IThirdResponse
    {
        /**
         * 获取查询结果数据。
         * @return 查询结果数据。
         */
        string data { get; }

        /**
         * 获取状态码。
         * @return 状态码。
         * 300 请求超时
         * 301 未连接
         * 302 连接不成功或者已断开
         * 303 uuid为空
         * 304 topic不能为空
         * 305 请求不能填入topic
         */
        int code { get; }
    
        /**
         * 获取消息。
         * @return 消息。
         */
        string msg { get; }

        /**
         * 获取请求ID。
         * @return 请求ID。
         */
        string uuid { get; }

        // 发送时间节点
        DateTime sendtime { get; set; }

        // 接受时间节点
        DateTime recvtime { get; set; }
    }
}
