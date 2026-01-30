using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1.Protocol
{
    public interface IResponseTime
    {
        // 发送时间节点
        DateTime sendtime { get; set; }

        // 接受时间节点
        DateTime recvtime { get; set; }
    }
}
