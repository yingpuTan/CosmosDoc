using Cosmos.DataAccess.v1.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.App.Hithink.ComDemo
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
