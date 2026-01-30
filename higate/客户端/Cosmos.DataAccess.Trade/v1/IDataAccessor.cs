using System.Globalization;

namespace Cosmos.DataAccess.Trade.v1
{
    /// <summary>
    /// Cosmos数据访问器
    /// </summary>
    public interface ITradeDataAccessor
    {
        /// <summary>
        /// 访问器的区域信息
        /// </summary>
        CultureInfo CultureInfo { get; set; }

        /// <summary>
        /// 会话控制器
        /// </summary>
        IDataSessionController DataSessionController { get; }

        /// <summary>
        /// 数据供应器
        /// </summary>
        ITradeDataProvider DataProvider { get; }


        /// <summary>
        /// 数据推理器
        /// </summary>
        IAIInferencer IAIInferencer { get; }

    }
}