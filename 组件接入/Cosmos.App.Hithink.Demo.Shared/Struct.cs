using Cosmos.DataAccess.v1.Model;

namespace Cosmos.App.Hithink.Demo.Shared
{
    /// <summary>
    /// 资产搜索结果记录
    /// </summary>
    public class AssetIdAndName
    {
        public IAssetId AssetId { get; set; }
        public string DisplayAssetId { get { return AssetId.SymbolValue; } }
        public string AssetName { get; set; }
        public string DisplayMarketName { get; set; }
    }
}

