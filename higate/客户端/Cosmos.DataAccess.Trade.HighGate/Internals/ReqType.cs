using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.Internals
{
    public enum ReqType
    {
        RT_PUSH,                /// 推送

        //Session相关请求
        RT_SESSION_BEGIN = 1000,
        
        RT_SESSION_USER_LOGIN,  /// 用户登录
    }
}
