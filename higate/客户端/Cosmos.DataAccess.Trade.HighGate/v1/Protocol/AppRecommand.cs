using Cosmos.DataAccess.Trade.HighGate.Internals.Protocol;
using Cosmos.DataAccess.Trade.HighGate.v1.Model;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Metadata;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1.Protocol
{
    /// <summary>
    /// 应用推送
    /// </summary>
    public class AppRecommand : IAppRecommand
    {
        public AppRecommand(DataResponse dataResponse)
        {
            var jsResult = JObject.Parse(dataResponse.response);
            if(jsResult != null)
            {
                AppGuid = jsResult["appId"].ToString();
                AppName = jsResult["appName"].ToString();
                Content = jsResult["content"].ToString();
            }
        }
        
        public string AppGuid { get; set; }

        public string AppName { get; set; }

        public string Content { get; set; }
    }

    /// <summary>
    /// 报警提示
    /// </summary>
    public class WarningTip : IWarningTip
    {
        public WarningTip(DataResponse dataResponse)
        {
            var jsResult = JObject.Parse(dataResponse.response);
            if (jsResult != null)
            {
                Content = jsResult["content"].ToString();
            }
        }

        public string Content { get; set; }
    }
}
