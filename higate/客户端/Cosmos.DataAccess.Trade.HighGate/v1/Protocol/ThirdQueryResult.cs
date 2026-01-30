using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    public class ThirdRequestResult : IThirdResponse
    {
        public string data {set; get;}

        public int code {set; get;}

        public string msg {set; get;}

        public string uuid { set; get; }

        // 发送时间节点
        public DateTime sendtime { get; set; }

        // 接受时间节点
        public DateTime recvtime { get; set; }
    }
}
