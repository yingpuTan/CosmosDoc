using Cosmos.DataAccess.Trade.HighGate.Internals;
using Cosmos.DataAccess.Trade.HighGate.v1.Model;
using Cosmos.DataAccess.Trade.HighGate.v1.Protocol;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using Cosmos.Observability.Application.Contracts.Logger;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.v1
{
    /// <summary>
    /// 数据供应器抽象
    /// 该接口定义了与交易数据相关的各种参数创建方法和查询接口。
    /// </summary>
    public class DataProvider: ITradeDataProvider
    {

        public DataProvider(
            ICosmosLogger cosmosLogger,
            IDataSessionController sessionController,
            DataCenter dataCenter)
        {
            _cosmosLogger = cosmosLogger;
            _sessionController = sessionController;
            _dataCenter = dataCenter;
        }

        #region 符号转换

        #endregion

        #region 创建参数

        /// <summary>
        /// 创建登陆资金账号参数
        /// </summary>
        /// <returns>返回一个IAccountLoginParameters实例</returns>
        public IAccountLoginParameters CreateAccountLoginParameters()
        {
            return new AccountLoginParameters();
        }

        /// <summary>
        /// 创建下单请求参数
        /// </summary>
        /// <returns>返回一个IOrderParameters实例</returns>
        public IOrderParameters CreateOrderParameters()
        {
            return new OrderParameters();
        }

        /// <summary>
        /// 创建撤单请求参数
        /// </summary>
        /// <returns>返回一个ICancelOrderParameters实例</returns>
        public ICancelOrderParameters CreateCancelOrderParameters()
        {
            return new CancelOrderParameters();
        }

        /// <summary>
        /// 创建查询银行流水请求参数
        /// </summary>
        /// <returns>返回一个IQueryBankFlowParameters实例</returns>
        public IQueryBankFlowParameters CreateQueryBankFlowParameters()
        {
            return new QueryBankFlowParameters();
        }

        /// <summary>
        /// 创建查询银行资金请求参数
        /// </summary>
        /// <returns>返回一个IQueryBankFundParameters实例</returns>
        public IQueryBankFundParameters CreateQueryBankFundParameters()
        {
            return new QueryBankFundParameters();
        }

        /// <summary>
        /// 创建查询银行请求参数
        /// </summary>
        /// <returns>返回一个IQueryBankParameters实例</returns>
        public IQueryBankParameters CreateQueryBankParameters()
        {
            return new QueryBankParameters();
        }

        /// <summary>
        /// 创建查询成交参数
        /// </summary>
        /// <returns>返回一个IQueryDealParameters实例</returns>
        public IQueryDealParameters CreateQueryDealParameters()
        {
            return new QueryDealParameters();
        }

        /// <summary>
        /// 创建查询退市信息请求参数
        /// </summary>
        /// <returns>返回一个IQueryDelistParameters实例</returns>
        public IQueryDelistParameters CreateQueryDelistParameters()
        {
            return new QueryDelistParameters();
        }

        /// <summary>
        /// 创建查询资金参数
        /// </summary>
        /// <returns>返回一个IQueryFundParameters实例</returns>
        public IQueryFundParameters CreateQueryFundParameters()
        {
            return new QueryFundParameters();
        }

        /// <summary>
        /// 创建查询资金流水请求参数
        /// </summary>
        /// <returns>返回一个IQueryFundsFlowParameters实例</returns>
        public IQueryFundsFlowParameters CreateQueryFundsFlowParameters()
        {
            return new QueryFundsFlowParameters();
        }

        /// <summary>
        /// 创建查询新股请求参数
        /// </summary>
        /// <returns>返回一个IQueryIpoParameters实例</returns>
        public IQueryIpoParameters CreateQueryIpoParameters()
        {
            return new QueryIpoParameters();
        }

        /// <summary>
        /// 创建查询配售额度请求参数
        /// </summary>
        /// <returns>返回一个IQueryIpoSharesParameters实例</returns>
        public IQueryIpoSharesParameters CreateQueryIpoSharesParameters()
        {
            return new QueryIpoSharesParameters();
        }

        /// <summary>
        /// 创建查询新股申购中签信息参数
        /// </summary>
        /// <returns>返回一个IQueryIpoSuccessParameters实例</returns>
        public IQueryIpoSuccessParameters CreateQueryIpoSuccessParameters()
        {
            return new QueryIpoSuccessParameters();
        }

        /// <summary>
        /// 创建查询委托参数
        /// </summary>
        /// <returns>返回一个IQueryOrderParameters实例</returns>
        public IQueryOrderParameters CreateQueryOrderParameters()
        {
            return new QueryOrderParameters();
        }

        /// <summary>
        /// 创建查询持仓参数
        /// </summary>
        /// <returns>返回一个IQueryPositionParameters实例</returns>
        public IQueryPositionParameters CreateQueryPositionParameters()
        {
            return new QueryPositionParameters();
        }

        /// <summary>
        /// 创建etf申购参数
        /// </summary>
        /// <returns>返回一个IETFOrderParameters实例</returns>
        public IETFOrderParameters CreateETFOrderParameters()
        {
            return new ETFOrderParameters();
        }

        /// <summary>
        /// 创建etf赎回参数
        /// </summary>
        /// <returns>返回一个IETFRedeemParameters实例</returns>
        public IETFRedeemParameters CreateETFRedeemParameters()
        {
            return new ETFRedeemParameters();
        }

        /// <summary>
        /// 创建etf盘后申购参数
        /// </summary>
        /// <returns>返回一个IETFOrderOffTimeParameters实例</returns>
        public IETFOrderOffTimeParameters CreateETFOrderOffTimeParameters()
        {
            return new ETFOrderOffTimeParameters();
        }

        /// <summary>
        /// 创建etf盘后赎回参数
        /// </summary>
        /// <returns>返回一个IETFRedeemOffTimeParameters实例</returns>
        public IETFRedeemOffTimeParameters CreateETFRedeemOffTimeParameters()
        {
            return new ETFRedeemOffTimeParameters();
        }

        /// <summary>
        /// 创建查询etf委托参数
        /// </summary>
        /// <returns>返回一个IQueryETFOrderParameters实例</returns>
        public IQueryETFOrderParameters CreateQueryETFOrderParameters()
        {
            return new QueryETFOrderParameters();
        }

        /// <summary>
        /// 创建第三方对接模块请求参数
        /// </summary>
        /// <returns>返回一个IQueryETFOrderParameters实例</returns>
        public IThirdRequestParameters CreateThirdRequestParameters()
        {
            return new ThirdQueryParameters();
        }

        /// <summary>
        /// 创建订阅具体详情信息参数
        /// </summary>
        /// <returns></returns>
        public ISubscribeReqMsg CreateSubscribeReqMsg()
        {
            return new SubscribeReqMsg();
        }

        /// <summary>
        /// 创建订阅请求参数
        /// </summary>
        /// <returns></returns>
        public ISubscribeParameters CreateSubscribeParameters()
        {
            return new SubscribeParameters();
        }

        /// <summary>
        /// 创建取消订阅请求参数
        /// </summary>
        /// <returns></returns>
        public IUnSubscribeParameters CreateUnSubscribeParameters()
        {
            return new UnSubscribeParameters();
        }
        #endregion

        #region 查询接口
        /// <summary>
        /// 资金账户登录
        /// </summary>
        /// <param name="parameters">登录请求参数</param>
        /// <returns>返回一个IAccountLoginResult实例</returns>
        public Task<IAccountLoginResult> AccountLogin(IAccountLoginParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 下单
        /// </summary>
        /// <param name="parameters">下单请求参数</param>
        /// <returns>返回一个IOrderResult实例</returns>
        public Task<IOrderResult> Order(IOrderParameters parameters)
        {
            throw new NotImplementedException();
        }


        /// <summary>
        /// 撤单
        /// </summary>
        /// <param name="parameters">撤单请求参数</param>
        /// <returns>返回一个ICancelOrderResult实例</returns>
        public Task<ICancelOrderResult> CancelOrder(ICancelOrderParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询银行流水。
        /// </summary>
        /// <param name="parameters">查询银行流水的参数。</param>
        /// <returns>返回银行流水查询结果。</returns>
        public Task<IQueryBankFlowResult> QueryBankFlow(IQueryBankFlowParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询银行资金。
        /// </summary>
        /// <param name="parameters">查询银行资金的参数。</param>
        /// <returns>返回银行资金查询结果。</returns>
        public Task<IQueryBankFundResult> QueryBankFund(IQueryBankFundParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询银行信息。
        /// </summary>
        /// <param name="parameters">查询银行信息的参数。</param>
        /// <returns>返回银行信息查询结果。</returns>
        public Task<IQueryBankResult> QueryBank(IQueryBankParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询交易信息。
        /// </summary>
        /// <param name="parameters">查询交易信息的参数。</param>
        /// <returns>返回交易信息查询结果。</returns>
        public Task<IQueryDealResult> QueryDeal(IQueryDealParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询退市信息。
        /// </summary>
        /// <param name="parameters">查询退市信息的参数。</param>
        /// <returns>返回退市信息查询结果。</returns>
        public Task<IQueryDelistResult> QueryDelist(IQueryDelistParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询基金信息。
        /// </summary>
        /// <param name="parameters">查询基金信息的参数。</param>
        /// <returns>返回基金信息查询结果。</returns>
        public Task<IQueryFundResult> QueryFund(IQueryFundParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询资金流向。
        /// </summary>
        /// <param name="parameters">查询资金流向的参数。</param>
        /// <returns>返回资金流向查询结果。</returns>
        public Task<IQueryFundsFlowResult> QueryFundsFlow(IQueryFundsFlowParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询IPO信息。
        /// </summary>
        /// <param name="parameters">查询IPO信息的参数。</param>
        /// <returns>返回IPO信息查询结果。</returns>
        public Task<IQueryIpoResult> QueryIpo(IQueryIpoParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询IPO配售信息。
        /// </summary>
        /// <param name="parameters">查询IPO配售信息的参数。</param>
        /// <returns>返回IPO配售信息查询结果。</returns>
        public Task<IQueryIpoSharesResult> QueryIpoShares(IQueryIpoSharesParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询IPO成功信息。
        /// </summary>
        /// <param name="parameters">查询IPO成功信息的参数。</param>
        /// <returns>返回IPO成功信息查询结果。</returns>
        public Task<IQueryIpoSuccessResult> QueryIpoSuccess(IQueryIpoSuccessParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询订单信息。
        /// </summary>
        /// <param name="parameters">查询订单信息的参数。</param>
        /// <returns>返回订单信息查询结果。</returns>
        public Task<IQueryOrderResult> QueryOrder(IQueryOrderParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询持仓信息。
        /// </summary>
        /// <param name="parameters">查询持仓信息的参数。</param>
        /// <returns>返回持仓信息查询结果。</returns>
        public Task<IQueryPositionResult> QueryPosition(IQueryPositionParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 执行ETF申购。
        /// </summary>
        /// <param name="parameters">ETF申购的参数。</param>
        /// <returns>返回ETF申购结果。</returns>
        public Task<IETFOrderResult> ETFOrder(IETFOrderParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 执行ETF赎回。
        /// </summary>
        /// <param name="parameters">ETF赎回的参数。</param>
        /// <returns>返回ETF赎回结果。</returns>
        public Task<IETFRedeemResult> ETFRedeem(IETFRedeemParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 查询ETF订单信息。
        /// </summary>
        /// <param name="parameters">查询ETF订单信息的参数。</param>
        /// <returns>返回ETF订单信息查询结果。</returns>
        public Task<IQueryETFOrderResult> QueryETFOrder(IQueryETFOrderParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 执行非交易时间内的ETF申购。
        /// </summary>
        /// <param name="parameters">非交易时间内的ETF申购参数。</param>
        /// <returns>返回非交易时间内的ETF申购结果。</returns>
        public Task<IETFOrderOffTimeResult> ETFOrderOffTime(IETFOrderOffTimeParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 执行非交易时间内的ETF赎回。
        /// </summary>
        /// <param name="parameters">非交易时间内的ETF赎回参数。</param>
        /// <returns>返回非交易时间内的ETF赎回结果。</returns>
        public Task<IETFRedeemOffTimeResult> ETFRedeemOffTime(IETFRedeemOffTimeParameters parameters)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 执行非交易时间内的ETF赎回。
        /// </summary>
        /// <param name="parameters">非交易时间内的ETF赎回参数。</param>
        /// <returns>返回非交易时间内的ETF赎回结果。</returns>
        public async Task<IThirdResponse> ThirdModuleRequest(IThirdRequestParameters parameters)
        {
            return await _dataCenter.ThirdRequest(parameters);
        }

        /// <summary>
        /// 向第三方服务异步发送请求。
        /// </summary>
        /// <param name="parameters">请求参数</param>
        /// <returns>应答结果</returns>
        public Task ThirdModuleRequestAsync(IThirdRequestParameters parameters, EventHandler<IThirdResponse> callback)
        {
            return  _dataCenter.ThirdRequestAsync(parameters, callback);
        }

        /// <summary>
        /// 获取登录信息
        /// </summary>
        /// <returns>返回登录信息。</returns>
        public async Task<IBaseLoginInfo> GetBaseInfo()
        {
            return _dataCenter.GetBaseInfo();
        }
        #endregion

        #region 订阅接口
        /// <summary>
        /// 订阅应用推送
        /// </summary>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        /// <returns></returns>
        public async Task<ITradeDataSubscription> SubscribAppCommand(EventHandler<IAppRecommand> subscriber)
        {
            var dataPushJob = _dataCenter.SubscribAppCommand();
            dataPushJob.Pushed += (pushJob, data_response) =>
            {
                var quoteQueryResult = new AppRecommand(data_response);
                subscriber?.Invoke(this, quoteQueryResult);
            };
            var hevoSubscription = new TradeDataSubscription(new[] { dataPushJob });
            return hevoSubscription;
        }

        /// <summary>
        /// 订阅预警提醒
        /// </summary>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        public async Task<ITradeDataSubscription> SubscribWarningTip(EventHandler<IWarningTip> subscriber)
        {
            var dataPushJob = _dataCenter.SubscribWarningTip();
            dataPushJob.Pushed += (pushJob, data_response) =>
            {
                var quoteQueryResult = new WarningTip(data_response);
                subscriber?.Invoke(this, quoteQueryResult);
            };
            var hevoSubscription = new TradeDataSubscription(new[] { dataPushJob });
            return hevoSubscription;
        }

        /// <summary>
        /// 向第三方模块订阅数据
        /// </summary>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        public async Task<ITradeDataSubscription> SubscribThirdModule(ISubscribeParameters subscriberParameters, EventHandler<IPushData> subscriber)
        {
            var dataPushJob = await _dataCenter.SubscribThirdModule(subscriberParameters);
            if (dataPushJob != null)
            {
                dataPushJob.Pushed += (pushJob, data_response) =>
                {
                    var quoteQueryResult = new PushData()
                    {
                        data = data_response.response
                    };
                    subscriber?.Invoke(this, quoteQueryResult);
                };
            }
           
            var hevoSubscription = new TradeDataSubscription(new[] { dataPushJob })
            {
                code = dataPushJob.code,
                msg = dataPushJob.msg,
            };

            return hevoSubscription;
        }

        /// <summary>
        /// 向第三方模块取消订阅数据
        /// </summary>
        /// <param name="subscriber"></param>
        /// <returns></returns>
        public async Task<IUnSubscribeResult> UnSubscribThirdModule(IUnSubscribeParameters unsubscriberParameters, ITradeDataSubscription subscription)
        {
            return await _dataCenter.UnSubscribThirdModule(unsubscriberParameters, subscription);
        }
        #endregion

        private ICosmosLogger _cosmosLogger { get; }

        private IDataSessionController _sessionController { get; }

        private DataCenter _dataCenter { get; }
    }
}