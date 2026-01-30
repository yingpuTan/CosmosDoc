using Cosmos.DataAccess.Trade.HighGate.Internals;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.Observability.Application.Contracts.Logger;
using System.Globalization;

namespace Cosmos.DataAccess.Trade.HighGate.v1
{
    /// <summary>
    /// Cosmos数据访问器
    /// </summary>
    public class TradeDataAccessor : ITradeDataAccessor
    {
        public TradeDataAccessor(
           ICosmosLogger cosmosLogger,
           HighGateProductInfo ProductInfo)
        {
            var sessionController = new DataSessionController(cosmosLogger, ProductInfo);
           // sessionController.SetAccountAndPassword(account, pass);
            DataSessionController = sessionController;
            dataCenter = new DataCenter(sessionController);
            DataProvider = new DataProvider( cosmosLogger, DataSessionController, dataCenter);

        }

        /// <summary>
        /// 访问器的区域信息
        /// </summary>
        public CultureInfo CultureInfo { get; set; }

        /// <summary>
        /// 会话控制器
        /// </summary>
        public IDataSessionController DataSessionController { get; }

        private DataCenter dataCenter { get; }
        /// <summary>
        /// 数据推理器
        /// </summary>
        public IAIInferencer IAIInferencer { get; }


        public ITradeDataProvider DataProvider { get; }
    }
}