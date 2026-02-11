using Cosmos.App.Sdk.v1;

namespace Cosmos.App.Hithink.Demo.Shared
{
    /// <summary>
    /// 实现组件访问器供，提供给组件引擎调用
    /// </summary>
    public class ComDemoProvider : ICosmosAppAccessProvider
    {
        public ICosmosAppAccessor AppAccessor { get; set; }

        public ICosmosAppStatusSerializer AppStatusSerializer { get; set; }
    }
}

