using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace Cosmos.DataAccess.Trade.v1.CosmosIntegration
{
    /// <summary>
    /// 数据交互抽象，继承此接口以获得数据访问能力
    /// </summary>
    public interface ICosmosTradeDataInteraction
    {
        /// <summary>
        /// 访问器注入
        /// </summary>
        ICosmosTradeAccessorsInjection TradeAccessorsInjection { get; set; }
    }
}
