#include "CCosmosHostApi.h"
#include "platform.h"
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include <string>
#include <vector>
#include <cstring>

// Base64字符表
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/"
;

#define SAFE_DELETE(ptr) \
if (ptr)\
{\
    delete ptr;\
    ptr = nullptr;\
}

static std::string GetSdkLibraryName() {
#if defined(_WIN32)
    return "CosmosSDK.dll";
#else
    return "libCosmosSDK.so";
#endif
}

static std::string GetMainAppName() {
#if defined(_WIN32)
    return "Cosmos.MainApp.exe";
#else
    // 非 Windows 下通常没有 .exe 后缀；如实际引擎包仍带后缀，可自行改这里或做成配置项
    return "Cosmos.MainApp";
#endif
}


// 将3个字节的输入转换为4个Base64字符
std::string base64_encode(const std::vector<unsigned char>&in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_chars[((val << -valb) & 0x3F)]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

// 将3个字节的输入转换为4个Base64字符
std::string base64_encode(const std::string & in) {
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_chars[((val << -valb) & 0x3F)]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

static std::string gbk_to_utf8(const std::string& s) { return platform::gbk_to_utf8(s); }
static std::string utf8_to_gbk(const std::string& s) { return platform::utf8_to_gbk(s); }


CCosmosApi* CCosmosApi::m_instance = nullptr;
std::mutex CCosmosApi::m_mutex;

#pragma region 以下方法为注册给cosmos引擎调用的方法

Cosmos_Result* Cosmos_Notify_Callback(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest)
{
    return CCosmosApi::GetInstance()->Cosmos_Notify(callContext, notifyRequest);
}

void Cosmos_ReleaseResult_Callback(const Cosmos_CallContext* callContext, const Cosmos_Result* result)
{
    CCosmosApi::GetInstance()->Cosmos_ReleaseResult(callContext, result);
}

Cosmos_InvokeResponse* Cosmos_Invoke_Callback(const Cosmos_CallContext* callContext, const Cosmos_InvokeRequest* invokeRequest)
{
    return CCosmosApi::GetInstance()->Cosmos_Invoke(callContext, invokeRequest);
}

void Cosmos_ReleaseInvokeResponse_Callback(const Cosmos_CallContext* callContext, const Cosmos_InvokeResponse* invokeResponse)
{
    CCosmosApi::GetInstance()->Cosmos_ReleaseInvokeResponse(callContext, invokeResponse);
}

Cosmos_SubscribeResponse* Cosmos_Subscribe_Callback(const Cosmos_CallContext* callContext, const Cosmos_SubscribeRequest* subscribeRequest)
{
    return CCosmosApi::GetInstance()->Cosmos_Subscribe(callContext, subscribeRequest);
}

Cosmos_Result* Cosmos_UnSubscribe_Callback(const Cosmos_CallContext* callContext, const Cosmos_Subscription* subscription)
{
    return CCosmosApi::GetInstance()->Cosmos_UnSubscribe(callContext, subscription);
}

Cosmos_Result* Cosmos_PushSubscriptionData_Callback(const Cosmos_CallContext* callContext, const Cosmos_SubscriptionDataFrame* subscriptionDataFrame)
{
    return CCosmosApi::GetInstance()->Cosmos_PushSubscriptionData(callContext, subscriptionDataFrame);
}

void Cosmos_ReleaseSubscribeResponse_Callback(const Cosmos_CallContext* callContext, const Cosmos_SubscribeResponse* subscriptionResponse)
{
    CCosmosApi::GetInstance()->Cosmos_ReleaseSubscribeResponse(callContext, subscriptionResponse);
}

#pragma endregion

void CCosmosApi::RegistNotify(notifyFunc func)
{
    if (func)
    {
        m_vecNotifySub.push_back(func);
    }
}


void CCosmosApi::Close()
{
    SAFE_DELETE(m_instance);
}
int uuid = 11111111;
void CCosmosApi::SimulatePush()
{
    for (auto subInfo:m_mapSubList)
    {
        std::string strTopic = subInfo.first;
        //模拟推送用户id变化的推送
        if (strTopic == "SubUserID")
        {
            uuid++;
            //组装推送包格式
			Cosmos_SubscriptionDataFrame* pPushInfo = new Cosmos_SubscriptionDataFrame;
            auto dataframe = new Cosmos_DataFrame();

            rapidjson::Document doc;
            doc.SetObject();
            doc.AddMember(rapidjson::StringRef("UserID"), rapidjson::Value(std::to_string(uuid).c_str(), doc.GetAllocator()), doc.GetAllocator());
            rapidjson::StringBuffer strBuf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
            doc.Accept(writer);
            std::string strReuslt = gbk_to_utf8(strBuf.GetString());

            dataframe->Data = strReuslt.c_str();		//应答数据
            dataframe->DataSize = strReuslt.length();	//应答数据长度
            pPushInfo->DataFrame = dataframe;

            //遍历订阅列表，发送推送信息
            for (auto subscribeid:subInfo.second)
            {
                pPushInfo->SubscriptionId = subscribeid.c_str();
                m_pPush(NULL, pPushInfo);
            }
        }
        //处理其他订阅。。。
        else
        {

        }
    }
}

CCosmosApi::CCosmosApi()
{
    std::string strPath = platform::executable_dir();
    std::string strSdkPath = platform::path_join(platform::path_join(strPath, "Cosmos"), GetSdkLibraryName());

    // 加载并校验宿主sdk函数指针
    platform::DynamicLibrary sdkModule;
    if (!sdkModule.load(strSdkPath))
    {
        return;
    }

    /// 获取cosmos引擎提供方法
    auto Call_Cosmos_InitializeEnvironment =
        reinterpret_cast<Cosmos_InitializeEnvironmentDelegate>(sdkModule.symbol("Cosmos_InitializeEnvironment")); //初始化环境变量函数
    m_pRelease =
        reinterpret_cast<Cosmos_UninitializeEnvironmentDelegate>(sdkModule.symbol("Cosmos_UninitializeEnvironment")); //去初始化环境变量，宿主关闭时使用

    /// 保存cosmos引擎提供的回调函数，集体代表含义，参考成员变量声明
    m_pNotify = reinterpret_cast<Cosmos_NotifyDelegate>(sdkModule.symbol("Cosmos_Notify"));
    m_pNotifyRelease = reinterpret_cast<Cosmos_ReleaseResultDelegate>(sdkModule.symbol("Cosmos_ReleaseResult"));
    m_pInvoke = reinterpret_cast<Cosmos_InvokeDelegate>(sdkModule.symbol("Cosmos_Invoke"));
    m_pInvokeRelease = reinterpret_cast<Cosmos_ReleaseInvokeResponseDelegate>(sdkModule.symbol("Cosmos_ReleaseInvokeResponse"));
    m_pSubscribe = reinterpret_cast<Cosmos_SubscribeDelegate>(sdkModule.symbol("Cosmos_Subscribe"));
    m_pSubscribeRelease = reinterpret_cast<Cosmos_ReleaseSubscribeResponseDelegate>(sdkModule.symbol("Cosmos_ReleaseSubscribeResponse"));
    m_pPush = reinterpret_cast<Cosmos_PushSubscriptionDataDelegate>(sdkModule.symbol("Cosmos_PushSubscriptionData"));
    m_pUnsubscribe = reinterpret_cast<Cosmos_UnsubscribeDelegate>(sdkModule.symbol("Cosmos_Unsubscribe"));

    //初始化客户端参数
    auto clientParameters = new Cosmos_ClientParameters;
    memset(clientParameters, 0, sizeof(Cosmos_ClientParameters));
    static std::string mainAppPath = platform::path_join("./Cosmos", GetMainAppName());
    clientParameters->CosmosMainAppPath = (char*)(mainAppPath.c_str());     //设置Cosmos引擎所在位置(可以填绝对位置)

    //设置行情账户信息和宿主账户信息
	rapidjson::Document docProduct, docMarketAccount;
    std::string dataProduct, dataMarketAccount;
    //设置宿主账户信息
    {
		docProduct.SetObject();
		docProduct.AddMember(rapidjson::StringRef("Account"), "test", docProduct.GetAllocator());                           //当前宿主的账号
        docProduct.AddMember(rapidjson::StringRef("Token"), "123", docProduct.GetAllocator());                              //本次登录的
        docProduct.AddMember(rapidjson::StringRef("Password"), "123123", docProduct.GetAllocator());                        //密码
		docProduct.AddMember(rapidjson::StringRef("ProductID"), "test", docProduct.GetAllocator());                         //当前宿主的产品id

		//下面三个配置建议设置成配置项读取，部署在不同环境的信息会发生变化
		docProduct.AddMember(rapidjson::StringRef("SpiderUrl"), "https://unitetest.chinastock.com.cn:8081", docProduct.GetAllocator());      //应用市场采集用户行为地址
		docProduct.AddMember(rapidjson::StringRef("Ip"), "10.4.124.34", docProduct.GetAllocator());                         //消息中心的ip
		docProduct.AddMember(rapidjson::StringRef("Port"), 9999, docProduct.GetAllocator());                              //消息中心的端口

		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
		docProduct.Accept(writer);
		dataProduct = gbk_to_utf8(strBuf.GetString());
		dataProduct = base64_encode(dataProduct);
    }

    //设置行情账户信息,现在账号不进行校验，可以随便填入
    {
        docMarketAccount.SetObject();
        docMarketAccount.AddMember(rapidjson::StringRef("Account"), "test", docProduct.GetAllocator());                           //行情账号
        docMarketAccount.AddMember(rapidjson::StringRef("Md5"), "123", docProduct.GetAllocator());                                //行情账号密码的md5值
        
		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        docMarketAccount.Accept(writer);
        dataMarketAccount = gbk_to_utf8(strBuf.GetString());
        dataMarketAccount = base64_encode(dataMarketAccount);
    }

    std::string strId = "HostDemo-```"+ dataMarketAccount + "```" + dataProduct + "```";
    clientParameters->Id = strId.c_str();                                       //产品id，改字段由三部分组成宿主的产品id + 行情账户信息 + 宿主账户信息，参数之间使用"```"做分隔                                  
    clientParameters->Version = "0.0.0.1";                                      //设置客户端版本号                              
    clientParameters->HighDpiMode = Comos_GuiHighDpiMode::SystemAware;          //高清屏配置                                            

    //开发者参数
    auto developerParameters = new Cosmos_DeveloperParameter;
    memset(developerParameters, 0, sizeof(Cosmos_DeveloperParameter));
    developerParameters->AppProviderMode = "nuget;https://unitetest.chinastock.com.cn:453/v3/index.json";  //设置组件来源,当为nuget时，分号后面的路径为应用市场访问地址
    developerParameters->RuntimeMode = "debug";                               //运行模式（Cosmos引擎内部使用，开发时选择debug、发行版本选择release）
    developerParameters->GuiMode = "show";                                      //应用商店是否显示（改应用商店非用户使用的应用商店，开发时可选择show、发行版本选择hide）

    //cef参数参数设置，如果存在cef的组件，需要设置该配置，不然无法运行cef组件
    auto webViewParameters = new Cosmos_WebViewParameters;
    memset(webViewParameters, 0, sizeof(Cosmos_WebViewParameters));
    webViewParameters->CefDirectory = "C:/Users/ThsQstudio";                         		   //cef动态库路径
    webViewParameters->CefResourcesDirectory = "C:/Users/ThsQstudio/Resources";                //cef资源路径
    webViewParameters->CefLocaleDirectory = "C:/Users/ThsQstudio/Resources/locales";           //cef字体包路径

    //向cosmos引擎提供宿主回调函数
    auto responsibility = new Cosmos_Responsibility;
    responsibility->Cosmos_NotifyHandler = Cosmos_Notify_Callback;				//向Cosmos引擎注册notify函数	
    responsibility->Cosmos_ReleaseResultHandler = Cosmos_ReleaseResult_Callback;
    responsibility->Cosmos_InvokeHandler = Cosmos_Invoke_Callback;				//向Cosmos引擎注册invoke函数	
    responsibility->Cosmos_ReleaseInvokeResponseHandler = Cosmos_ReleaseInvokeResponse_Callback;
    responsibility->Cosmos_SubscribeHandler = Cosmos_Subscribe_Callback;		//向Cosmos引擎注册订阅函数
    responsibility->Cosmos_UnsubscribeHandler = Cosmos_UnSubscribe_Callback;		//向Cosmos引擎注册取消订阅函数
    responsibility->Cosmos_PushSubscriptionDataHandler = Cosmos_PushSubscriptionData_Callback;
    responsibility->Cosmos_ReleaseSubscribeResponseHandler = Cosmos_ReleaseSubscribeResponse_Callback;

	//设置cosmos引擎的环境参数
    m_pEnvironment = new Cosmos_EnvironmentCreationParameters;
    m_pEnvironment->Responsibility = responsibility;
    m_pEnvironment->ClientParameters = clientParameters;
    m_pEnvironment->DeveloperParameter = developerParameters;
    m_pEnvironment->WebViewParameters = webViewParameters;

    //调用Cosmos引擎初始化函数，启动Cosmos引擎
    auto pRes = Call_Cosmos_InitializeEnvironment(nullptr, m_pEnvironment, nullptr);
    if (pRes->Code == 200)
    {
        printf("启动成功\n");

        //启动成功，开启定时器，模拟订阅推送（跨平台后台线程）
        m_timer = std::make_unique<platform::PeriodicTimer>(std::chrono::milliseconds(100), []() {
            CCosmosApi::GetInstance()->SimulatePush();
        });
    }
    else
    {
        printf("启动失败：code：%d, msg:%s\n", pRes->Code, pRes->Message);
    }
}

CCosmosApi::~CCosmosApi()
{
    if (m_timer) m_timer->stop();
    SAFE_DELETE(m_pEnvironment->Responsibility);
    SAFE_DELETE(m_pEnvironment->ClientParameters);
    SAFE_DELETE(m_pEnvironment->DeveloperParameter);
    SAFE_DELETE(m_pEnvironment->WebViewParameters);
    SAFE_DELETE(m_pEnvironment);
    if (m_pRelease)
    {
		m_pRelease();
    }
}


std::string CCosmosApi::Invoke(const std::string& strMethod, const std::string& strRequest)
{
    std::string strReuslt ="";
    if (m_pInvoke)
    {
		std::string strUtf8Method = gbk_to_utf8(strMethod);
		std::string strUtfRequest = gbk_to_utf8(strRequest);
        Cosmos_InvokeRequest* pRequest = new Cosmos_InvokeRequest;

        pRequest->Method = strUtf8Method.c_str();
        pRequest->Parameters = strUtfRequest.c_str();
        auto pRes = m_pInvoke(nullptr, pRequest);

        if (pRes)
        {
            rapidjson::Document doc;
            doc.SetObject();
            doc.AddMember(rapidjson::StringRef("Code"), pRes->Result->Code, doc.GetAllocator());
            doc.AddMember(rapidjson::StringRef("Data"), rapidjson::StringRef(pRes->DataFrame->Data), doc.GetAllocator());
            
            rapidjson::StringBuffer strBuf;
            rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
            doc.Accept(writer);
            strReuslt = utf8_to_gbk(strBuf.GetString());

            //释放对方应答数据
            m_pInvokeRelease(nullptr, pRes);
        }
    }
    return strReuslt;
}

void CCosmosApi::Notify(const std::string& strTopic, const std::string& strNotify)
{
	std::string strUtf8Topic = gbk_to_utf8(strTopic);
	std::string strUtfNotify = gbk_to_utf8(strNotify);
    Cosmos_NotifyRequest *pNotify = new Cosmos_NotifyRequest;

    pNotify->Topic = strUtf8Topic.c_str();
    pNotify->Message = strUtfNotify.c_str();
    pNotify->RoutingKey = "";

    auto pRes = m_pNotify(nullptr, pNotify);
    if (pRes)
    {
        //释放内存
        m_pNotifyRelease(nullptr, pRes);
    }
}

//该方法组件引擎暂未实现
std::string CCosmosApi::SubScribe(const std::string& strTopic, const std::string& strSubscribe)
{
	std::string strUtf8Topic = gbk_to_utf8(strTopic);
	std::string strUtfSubscribe = gbk_to_utf8(strSubscribe);
    Cosmos_SubscribeRequest* pSub = new Cosmos_SubscribeRequest;
	pSub->Topic = strUtf8Topic.c_str();
	pSub->Parameters = strUtfSubscribe.c_str();
    std::string strUUid = "";

    auto pRes = m_pSubscribe(nullptr, pSub);
    if (pRes)
    {
        //订阅成功
        if (pRes->Result->Code == 200)
        {
            strUUid = pRes->Subscription->SubscriptionId;
        }
        //释放对方应答
        m_pSubscribeRelease(nullptr, pRes);
    }
    return strUUid;
}

Cosmos_Result* CCosmosApi::Cosmos_Notify(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest)
{
    for (auto func : m_vecNotifySub)
    {
        func(utf8_to_gbk(notifyRequest->Message));
    }

	rapidjson::Document docNotify;
	docNotify.Parse(utf8_to_gbk(notifyRequest->Message).c_str());

    if (!docNotify.HasParseError() && docNotify.HasMember("ActionContext") && docNotify["ActionContext"].HasMember("NotifyType"))
    {
        //收到组件设置名字的通知
        if (std::string(docNotify["ActionContext"]["NotifyType"].GetString()) == "SetName")
        {
            std::string strName = docNotify["ActionContext"]["Parameters"]["name"].GetString();
            printf("Recv Notify SetName Name:%s\n", strName.c_str());
        }
    }

    //通知消息对方不管应答，直接返回正确就行
    auto result = new Cosmos_Result;
    result->Code = 200;		//成功应答返回200、其他为错误
    result->Message = "";   //处理的结果
    return result;
}

void CCosmosApi::Cosmos_ReleaseResult(const Cosmos_CallContext* callContext, const Cosmos_Result* result)
{
    if (result)
    {
        SAFE_DELETE(result);
    }
}

Cosmos_InvokeResponse* CCosmosApi::Cosmos_Invoke(const Cosmos_CallContext* callContext, const Cosmos_InvokeRequest* invokeRequest)
{
	std::string strMethod = utf8_to_gbk(invokeRequest->Method);
	std::string strParam = utf8_to_gbk(invokeRequest->Parameters);
	rapidjson::Document docParam;
    docParam.Parse(strParam.c_str());

    auto result = new Cosmos_Result;
    result->Code = 200;			        //成功应答返回200、其他为错误
    result->Message = "success";		//处理的结果信息
	auto dataframe = new Cosmos_DataFrame;
	
    //处理组件获取用户id方法
    if (strMethod == "getUserID")
    {
		rapidjson::Document doc;
		doc.SetObject();
        doc.AddMember(rapidjson::StringRef("UserID"), "11111111", doc.GetAllocator());

		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
		doc.Accept(writer);
		std::string strReuslt = gbk_to_utf8(strBuf.GetString());
        int length = strReuslt.length();
        char* pData = new char[length + 1];
        memset(pData, 0, length + 1);
        std::memcpy(pData, strReuslt.c_str(), static_cast<size_t>(length));
        pData[length] = '\0';
		dataframe->Data = pData;		            //应答数据
		dataframe->DataSize = strReuslt.length();	//应答数据长度
    }
    //处理组件获取账户列表方法
    else if (strMethod == "getAccounts")
    {
        //应答串格式由宿主定义、组件根据宿主返回的格式进行解析
		rapidjson::Document doc;
        doc.SetArray();
        for (int i = 1; i < 11 ; i++)
        {
            rapidjson::Value jsTmp(rapidjson::kObjectType);
            std::string strIndex = std::to_string(i);
			std::string strAccountName = "Acount_" + strIndex;
			std::string strAccount = "1111111" + strIndex;
			std::string strQsid = "80";
			std::string strState = (i % 2) == 1 ? "login" : "faild";
            jsTmp.AddMember(rapidjson::StringRef("AccountName"), rapidjson::Value(strAccountName.c_str(), doc.GetAllocator()), doc.GetAllocator());
            jsTmp.AddMember(rapidjson::StringRef("Account"), rapidjson::Value(strAccount.c_str(), doc.GetAllocator()), doc.GetAllocator());
            jsTmp.AddMember(rapidjson::StringRef("Qsid"), rapidjson::Value(strQsid.c_str(), doc.GetAllocator()), doc.GetAllocator());
            jsTmp.AddMember(rapidjson::StringRef("State"), rapidjson::Value(strState.c_str(), doc.GetAllocator()), doc.GetAllocator());
            doc.PushBack(jsTmp, doc.GetAllocator());
        }
	
		rapidjson::StringBuffer strBuf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
		doc.Accept(writer);
		std::string strReuslt = gbk_to_utf8(strBuf.GetString());
        int length = strReuslt.length();
		char* pData = new char[length + 1];
		memset(pData, 0, length + 1);
        std::memcpy(pData, strReuslt.c_str(), static_cast<size_t>(length));
        pData[length] = '\0';
		dataframe->Data = pData;		            //应答数据
		dataframe->DataSize = strReuslt.length();	//应答数据长度
    }
	else
	{
        dataframe->Data = "";		                //应答数据
		dataframe->DataSize = 0;	                //应答数据长度
        result->Code = 501;                         //应答错误代码
		result->Message = "not support";		    //应答错误代码
	}
   
    auto response = new Cosmos_InvokeResponse;
    response->Result = result;
    response->DataFrame = dataframe;
    return response;
}

void CCosmosApi::Cosmos_ReleaseInvokeResponse(const Cosmos_CallContext* callContext, const Cosmos_InvokeResponse* invokeResponse)
{
    if (invokeResponse)
    {
        auto result = invokeResponse->Result;
        auto dataframe = invokeResponse->DataFrame;
        SAFE_DELETE(result);
        SAFE_DELETE(dataframe);
        SAFE_DELETE(invokeResponse);
    }
}

std::string CCosmosApi::GetUUid()
{
    return platform::uuid_v4();
}

//处理cosmos引擎发起的订阅请求
Cosmos_SubscribeResponse* CCosmosApi::Cosmos_Subscribe(const Cosmos_CallContext* callContext, const Cosmos_SubscribeRequest* subscribeRequest)
{
	auto result = new Cosmos_Result;
	result->Code = 200;				//成功应答返回200、其他为错误
	result->Message = "";			//处理的结果信息

	auto data = new Cosmos_Subscription;
    data->SubscriptionId = "";
	rapidjson::Document docNotify;
	docNotify.Parse(utf8_to_gbk(subscribeRequest->Parameters).c_str());
	if (!docNotify.HasParseError() && docNotify.HasMember("ActionContext") && docNotify["ActionContext"].HasMember("Function"))
	{
		//收到组件订阅userid变化消息
		if (std::string(docNotify["ActionContext"]["Function"].GetString()) == "SubUserID")
		{
            std::string strUUid = GetUUid();
			int length = strUUid.length();
			char* pData = new char[length + 1];
			memset(pData, 0, length + 1);
            std::memcpy(pData, strUUid.c_str(), static_cast<size_t>(length));
            pData[length] = '\0';
            m_mapSubList["SubUserID"].push_back(strUUid);

            //订阅成功，返回一个订阅id给订阅者
            data->SubscriptionId = pData;
		}
        else
        {
            result->Code = 500;				//不处理
            result->Message = "不识别的订阅请求";
        }
	}

    auto response = new Cosmos_SubscribeResponse;
    response->Result = result;
    response->Subscription = data;
    return response;
}

//处理cosmos引擎发起的取消订阅请求
Cosmos_Result* CCosmosApi::Cosmos_UnSubscribe(const Cosmos_CallContext* callContext, const Cosmos_Subscription* subscription)
{
    //找到id所在的topic，从该topic冲清楚订阅者
    if (m_mapIdToTopic.find(subscription->SubscriptionId) != m_mapIdToTopic.end())
    {
        std::string strTopic = m_mapIdToTopic[subscription->SubscriptionId];
        if(m_mapSubList.find(strTopic) != m_mapSubList.end())
        {
            auto vecSubList = m_mapSubList[strTopic];
            auto iter = std::find(vecSubList.begin(), vecSubList.end(), subscription->SubscriptionId);
            m_mapSubList[strTopic].erase(iter);
        }
    }
    return nullptr;
}

//暂未实现
Cosmos_Result* CCosmosApi::Cosmos_PushSubscriptionData(const Cosmos_CallContext* callContext, const Cosmos_SubscriptionDataFrame* subscriptionDataFrame)
{
	auto result = new Cosmos_Result;
	result->Code = 200;				//成功应答返回200、其他为错误
	result->Message = "";			//处理的结果信息


    return result;
}

//暂未实现
void CCosmosApi::Cosmos_ReleaseSubscribeResponse(const Cosmos_CallContext* callContext, const Cosmos_SubscribeResponse* subscriptionResponse)
{
    if (subscriptionResponse)
    {
        auto result = subscriptionResponse->Result;
        auto data = subscriptionResponse->Subscription;
        SAFE_DELETE(result);
        SAFE_DELETE(data);
        SAFE_DELETE(subscriptionResponse);
    }
}