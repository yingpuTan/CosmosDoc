using Cosmos.App.Sdk.v1;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.App.Hithink.ComDemo
{
    /// <summary>
    /// 实现组件访问器供，提供给组件引擎调用
    /// </summary>
    internal class ComDemoProvider : ICosmosAppAccessProvider
    {
        public ICosmosAppAccessor AppAccessor { get; set; }

        public ICosmosAppStatusSerializer AppStatusSerializer { get; set; }
    }

}
