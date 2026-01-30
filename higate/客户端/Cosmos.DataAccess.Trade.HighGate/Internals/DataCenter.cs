using Cosmos.DataAccess.Trade.HighGate.Internals.Protocol;
using Cosmos.DataAccess.Trade.HighGate.v1;
using Cosmos.DataAccess.Trade.v1;
using Cosmos.DataAccess.Trade.v1.Model;
using Cosmos.DataAccess.Trade.v1.Protocol;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Cosmos.DataAccess.Trade.HighGate.Internals
{
    public class DataCenter
    {
        public DataCenter(DataSessionController dataSessionController)
        {
            _dataSessionController = dataSessionController;
        }

        public DataPushJob SubscribAppCommand()
        {
            return _dataSessionController?.SubScribe("upb_index_cosmos_quant_cosmos");
        }

        public DataPushJob SubscribWarningTip()
        {
            return _dataSessionController?.SubScribe("upb_cosmos_waring_tip");
        }

        public async Task<DataPushJob> SubscribThirdModule(ISubscribeParameters subscriberParameters)
        {
            return await _dataSessionController?.ThirdSubsribe(subscriberParameters);
        }

        public async Task<IUnSubscribeResult> UnSubscribThirdModule(IUnSubscribeParameters unsubscriberParameters, ITradeDataSubscription subscription)
        {
            return await _dataSessionController?.ThirdUnSubsribe(unsubscriberParameters, subscription);
        }
        public IBaseLoginInfo GetBaseInfo()
        {
            return _dataSessionController?.GetBaseInfo();
        }

        public async Task<IThirdResponse> ThirdRequest(IThirdRequestParameters parameters)
        {
            return await _dataSessionController?.ThirdRequest(parameters);
        }

        public Task ThirdRequestAsync(IThirdRequestParameters parameters, EventHandler<IThirdResponse> callback)
        {
            return _dataSessionController?.ThirdRequestAsync(parameters, callback);
        }

        private DataSessionController _dataSessionController { get; }
    }
}
