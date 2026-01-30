using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.Internals.Protocol
{
    public class DataSubscription
    {
        public string Topic { get; set; }
        public DataPushJob PushJob { get; set; }
    }
}
