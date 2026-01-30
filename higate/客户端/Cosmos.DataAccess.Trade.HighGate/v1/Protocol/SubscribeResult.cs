using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    public class SubscribeResult : ISubscribeResult
    {
        public string topic { get; set; }

        public IList<ISubscribeReqMsg> rsp { get; set; }
    }


    public class UnSubscribeResult :IUnSubscribeResult
    {
         public int code { get; set; }

         public string msg { get; set; }
    }
}
