namespace Cosmos.DataAccess.Trade.v1.CosmosIntegration
{
    /// <summary>
    /// Cosmos数据访问注入接口，请Widget作者继承以获得被注入数据访问能力
    /// </summary>
    public interface ICosmosTradeAccessorsInjection
    {
        /// <summary>
        /// 数据访问器注入
        /// </summary>
        ITradeDataAccessor DataAccessor { get; set; }
    }
}
