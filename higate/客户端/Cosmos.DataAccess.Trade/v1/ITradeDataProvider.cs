using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.v1
{
    /// <summary>
    /// 数据供应器抽象
    /// 该接口定义了与交易数据相关的各种参数创建方法和查询接口。
    /// </summary>
    public interface ITradeDataProvider
    {

        #region 符号转换

        #endregion

        #region 创建参数

        /// <summary>
        /// 创建登陆资金账号参数
        /// </summary>
        /// <returns>返回一个IAccountLoginParameters实例</returns>
        IAccountLoginParameters CreateAccountLoginParameters();

        /// <summary>
        /// 创建下单请求参数
        /// </summary>
        /// <returns>返回一个IOrderParameters实例</returns>
        IOrderParameters CreateOrderParameters();

        /// <summary>
        /// 创建撤单请求参数
        /// </summary>
        /// <returns>返回一个ICancelOrderParameters实例</returns>
        ICancelOrderParameters CreateCancelOrderParameters();

        /// <summary>
        /// 创建查询银行流水请求参数
        /// </summary>
        /// <returns>返回一个IQueryBankFlowParameters实例</returns>
        IQueryBankFlowParameters CreateQueryBankFlowParameters();

        /// <summary>
        /// 创建查询银行资金请求参数
        /// </summary>
        /// <returns>返回一个IQueryBankFundParameters实例</returns>
        IQueryBankFundParameters CreateQueryBankFundParameters();

        /// <summary>
        /// 创建查询银行请求参数
        /// </summary>
        /// <returns>返回一个IQueryBankParameters实例</returns>
        IQueryBankParameters CreateQueryBankParameters();

        /// <summary>
        /// 创建查询成交参数
        /// </summary>
        /// <returns>返回一个IQueryDealParameters实例</returns>
        IQueryDealParameters CreateQueryDealParameters();

        /// <summary>
        /// 创建查询退市信息请求参数
        /// </summary>
        /// <returns>返回一个IQueryDelistParameters实例</returns>
        IQueryDelistParameters CreateQueryDelistParameters();

        /// <summary>
        /// 创建查询资金参数
        /// </summary>
        /// <returns>返回一个IQueryFundParameters实例</returns>
        IQueryFundParameters CreateQueryFundParameters();

        /// <summary>
        /// 创建查询资金流水请求参数
        /// </summary>
        /// <returns>返回一个IQueryFundsFlowParameters实例</returns>
        IQueryFundsFlowParameters CreateQueryFundsFlowParameters();

        /// <summary>
        /// 创建查询新股请求参数
        /// </summary>
        /// <returns>返回一个IQueryIpoParameters实例</returns>
        IQueryIpoParameters CreateQueryIpoParameters();

        /// <summary>
        /// 创建查询配售额度请求参数
        /// </summary>
        /// <returns>返回一个IQueryIpoSharesParameters实例</returns>
        IQueryIpoSharesParameters CreateQueryIpoSharesParameters();

        /// <summary>
        /// 创建查询新股申购中签信息参数
        /// </summary>
        /// <returns>返回一个IQueryIpoSuccessParameters实例</returns>
        IQueryIpoSuccessParameters CreateQueryIpoSuccessParameters();

        /// <summary>
        /// 创建查询委托参数
        /// </summary>
        /// <returns>返回一个IQueryOrderParameters实例</returns>
        IQueryOrderParameters CreateQueryOrderParameters();

        /// <summary>
        /// 创建查询持仓参数
        /// </summary>
        /// <returns>返回一个IQueryPositionParameters实例</returns>
        IQueryPositionParameters CreateQueryPositionParameters();

        /// <summary>
        /// 创建etf申购参数
        /// </summary>
        /// <returns>返回一个IETFOrderParameters实例</returns>
        IETFOrderParameters CreateETFOrderParameters();

        /// <summary>
        /// 创建etf赎回参数
        /// </summary>
        /// <returns>返回一个IETFRedeemParameters实例</returns>
        IETFRedeemParameters CreateETFRedeemParameters();

        /// <summary>
        /// 创建etf盘后申购参数
        /// </summary>
        /// <returns>返回一个IETFOrderOffTimeParameters实例</returns>
        IETFOrderOffTimeParameters CreateETFOrderOffTimeParameters();

        /// <summary>
        /// 创建etf盘后赎回参数
        /// </summary>
        /// <returns>返回一个IETFRedeemOffTimeParameters实例</returns>
        IETFRedeemOffTimeParameters CreateETFRedeemOffTimeParameters();

        /// <summary>
        /// 创建查询etf委托参数
        /// </summary>
        /// <returns>返回一个IQueryETFOrderParameters实例</returns>
        IQueryETFOrderParameters CreateQueryETFOrderParameters();

        /// <summary>
        /// 创建第三方对接模块请求参数
        /// </summary>
        /// <returns>返回一个IQueryETFOrderParameters实例</returns>
        IThirdRequestParameters CreateThirdRequestParameters();

        /// <summary>
        /// 创建订阅具体详情信息参数
        /// </summary>
        /// <returns></returns>
        ISubscribeReqMsg CreateSubscribeReqMsg();

        /// <summary>
        /// 创建订阅请求参数
        /// </summary>
        /// <returns></returns>
        ISubscribeParameters CreateSubscribeParameters();

        /// <summary>
        /// 创建取消订阅请求参数
        /// </summary>
        /// <returns></returns>
        IUnSubscribeParameters CreateUnSubscribeParameters();
        #endregion

        #region 查询接口
        /// <summary>
        /// 资金账户登录
        /// </summary>
        /// <param name="parameters">登录请求参数</param>
        /// <returns>返回一个IAccountLoginResult实例</returns>
        Task<IAccountLoginResult> AccountLogin(IAccountLoginParameters parameters);

        /// <summary>
        /// 下单
        /// </summary>
        /// <param name="parameters">下单请求参数</param>
        /// <returns>返回一个IOrderResult实例</returns>
        Task<IOrderResult> Order(IOrderParameters parameters);

        /// <summary>
        /// 撤单
        /// </summary>
        /// <param name="parameters">撤单请求参数</param>
        /// <returns>返回一个ICancelOrderResult实例</returns>
        Task<ICancelOrderResult> CancelOrder(ICancelOrderParameters parameters);

        /// <summary>
        /// 查询银行流水。
        /// </summary>
        /// <param name="parameters">查询银行流水的参数。</param>
        /// <returns>返回银行流水查询结果。</returns>
        Task<IQueryBankFlowResult> QueryBankFlow(IQueryBankFlowParameters parameters);
        
        /// <summary>
        /// 查询银行资金。
        /// </summary>
        /// <param name="parameters">查询银行资金的参数。</param>
        /// <returns>返回银行资金查询结果。</returns>
        Task<IQueryBankFundResult> QueryBankFund(IQueryBankFundParameters parameters);
        
        /// <summary>
        /// 查询银行信息。
        /// </summary>
        /// <param name="parameters">查询银行信息的参数。</param>
        /// <returns>返回银行信息查询结果。</returns>
        Task<IQueryBankResult> QueryBank(IQueryBankParameters parameters);
        
        /// <summary>
        /// 查询交易信息。
        /// </summary>
        /// <param name="parameters">查询交易信息的参数。</param>
        /// <returns>返回交易信息查询结果。</returns>
        Task<IQueryDealResult> QueryDeal(IQueryDealParameters parameters);
        
        /// <summary>
        /// 查询退市信息。
        /// </summary>
        /// <param name="parameters">查询退市信息的参数。</param>
        /// <returns>返回退市信息查询结果。</returns>
        Task<IQueryDelistResult> QueryDelist(IQueryDelistParameters parameters);
        
        /// <summary>
        /// 查询基金信息。
        /// </summary>
        /// <param name="parameters">查询基金信息的参数。</param>
        /// <returns>返回基金信息查询结果。</returns>
        Task<IQueryFundResult> QueryFund(IQueryFundParameters parameters);
        
        /// <summary>
        /// 查询资金流向。
        /// </summary>
        /// <param name="parameters">查询资金流向的参数。</param>
        /// <returns>返回资金流向查询结果。</returns>
        Task<IQueryFundsFlowResult> QueryFundsFlow(IQueryFundsFlowParameters parameters);
        
        /// <summary>
        /// 查询IPO信息。
        /// </summary>
        /// <param name="parameters">查询IPO信息的参数。</param>
        /// <returns>返回IPO信息查询结果。</returns>
        Task<IQueryIpoResult> QueryIpo(IQueryIpoParameters parameters);
        
        /// <summary>
        /// 查询IPO配售信息。
        /// </summary>
        /// <param name="parameters">查询IPO配售信息的参数。</param>
        /// <returns>返回IPO配售信息查询结果。</returns>
        Task<IQueryIpoSharesResult> QueryIpoShares(IQueryIpoSharesParameters parameters);
        
        /// <summary>
        /// 查询IPO成功信息。
        /// </summary>
        /// <param name="parameters">查询IPO成功信息的参数。</param>
        /// <returns>返回IPO成功信息查询结果。</returns>
        Task<IQueryIpoSuccessResult> QueryIpoSuccess(IQueryIpoSuccessParameters parameters);
        
        /// <summary>
        /// 查询订单信息。
        /// </summary>
        /// <param name="parameters">查询订单信息的参数。</param>
        /// <returns>返回订单信息查询结果。</returns>
        Task<IQueryOrderResult> QueryOrder(IQueryOrderParameters parameters);
        
        /// <summary>
        /// 查询持仓信息。
        /// </summary>
        /// <param name="parameters">查询持仓信息的参数。</param>
        /// <returns>返回持仓信息查询结果。</returns>
        Task<IQueryPositionResult> QueryPosition(IQueryPositionParameters parameters);
        
        /// <summary>
        /// 执行ETF申购。
        /// </summary>
        /// <param name="parameters">ETF申购的参数。</param>
        /// <returns>返回ETF申购结果。</returns>
        Task<IETFOrderResult> ETFOrder(IETFOrderParameters parameters);
        
        /// <summary>
        /// 执行ETF赎回。
        /// </summary>
        /// <param name="parameters">ETF赎回的参数。</param>
        /// <returns>返回ETF赎回结果。</returns>
        Task<IETFRedeemResult> ETFRedeem(IETFRedeemParameters parameters);
        
        /// <summary>
        /// 查询ETF订单信息。
        /// </summary>
        /// <param name="parameters">查询ETF订单信息的参数。</param>
        /// <returns>返回ETF订单信息查询结果。</returns>
        Task<IQueryETFOrderResult> QueryETFOrder(IQueryETFOrderParameters parameters);
        
        /// <summary>
        /// 执行非交易时间内的ETF申购。
        /// </summary>
        /// <param name="parameters">非交易时间内的ETF申购参数。</param>
        /// <returns>返回非交易时间内的ETF申购结果。</returns>
        Task<IETFOrderOffTimeResult> ETFOrderOffTime(IETFOrderOffTimeParameters parameters);
        
       
        Task<IETFRedeemOffTimeResult> ETFRedeemOffTime(IETFRedeemOffTimeParameters parameters);

        /// <summary>
        /// 向第三方服务同步发送请求。
        /// </summary>
        /// <param name="parameters">请求参数</param>
        /// <returns>应答结果</returns>
        Task<IThirdResponse> ThirdModuleRequest(IThirdRequestParameters parameters);

        /// <summary>
        /// 向第三方服务异步发送请求。
        /// </summary>
        /// <param name="parameters">请求参数</param>
        /// <returns>应答结果</returns>
        Task ThirdModuleRequestAsync(IThirdRequestParameters parameters, EventHandler<IThirdResponse> callback);

        /// <summary>
        /// 获取登录信息
        /// </summary>
        /// <returns>返回登录信息。</returns>
        Task<IBaseLoginInfo> GetBaseInfo();
        #endregion

        #region 订阅接口
        /// <summary>
        /// 订阅应用推送
        /// </summary>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        Task<ITradeDataSubscription> SubscribAppCommand(EventHandler<IAppRecommand> subscriber);


        /// <summary>
        /// 订阅预警提醒
        /// </summary>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        Task<ITradeDataSubscription> SubscribWarningTip(EventHandler<IWarningTip> subscriber);


        /// <summary>
        /// 向第三方模块订阅数据
        /// </summary>
        /// <param name="subscriberParameters">订阅参数</param>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        Task<ITradeDataSubscription> SubscribThirdModule(ISubscribeParameters subscriberParameters, EventHandler<IPushData> subscriber);

        /// <summary>
        /// 向第三方模块取消订阅数据
        /// </summary>
        /// <param name="unsubscriberParameters"></param>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        Task<IUnSubscribeResult> UnSubscribThirdModule(IUnSubscribeParameters unsubscriberParameters, ITradeDataSubscription subscription);
        #endregion
    }
}