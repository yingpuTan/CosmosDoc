using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.Internals
{
    public class LoginResult
    {
        public bool is_payed { get; set; }

        public string permission { get; set; }

        public int retcode { get; set; }

        public string server_name { get; set; }

        public string time { get; set; }

        public string token { get; set; }

        public string userid { get; set; }

        public string username { get; set; }

    }

    public class SessionEvent
    {
        public static readonly string CONNECTING = "connecting";

        public static readonly string CONNECT_SUCCESS = "connect_success";

        public static readonly string CONNECT_FAIL = "connect_fail";

        public static readonly string DISCONNECT = "disconnnect";

        public static readonly string REDISCONNECTING = "reconnect";

        public static readonly string LOGINING = "logining";

        public static readonly string LOGIN_SUCCESS = "login_success";

        public static readonly string LOGIN_FAIL = "login_fail";

        public static readonly string KICK_USER = "kick_user";
    }
}
