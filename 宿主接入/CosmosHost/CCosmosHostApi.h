#pragma once
#include "Cosmos.Product.Sdk.h"
#include <mutex>
#include <vector>
#include <map>
#include <memory>

namespace platform {
class PeriodicTimer;
}

class CCosmosApi {
    using notifyFunc = void(*)(const std::string &);

public:
    static CCosmosApi* GetInstance() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new CCosmosApi();
        }
        return m_instance;
    }

    /// @brief 提供外部向Cosmos引擎发送请求
    /// @param strMethod 方法名
    /// @param strRequest 请求内容
    /// @return 调用结果
    std::string Invoke(const std::string &strMethod, const std::string &strRequest);

    /// @brief 提供外部向Cosmos引擎发送通知
    /// @param callContext 通知类型
    /// @param strNotify 通知内容
    void Notify(const std::string& strTopic, const std::string& strNotify);

	/// @brief 提供外部向Cosmos引擎请求订阅
	/// @param callContext 订阅主题
	/// @param strNotify 订阅参数
    /// @return 订阅成功id
    std::string SubScribe(const std::string& strTopic, const std::string& strSubscribe);

    /// @brief 处理Cosmos引擎发送的通知
    /// @param callContext 发送者信息
    /// @param notifyRequest 通知信息内容
    /// @return 通知处理结果
    Cosmos_Result* Cosmos_Notify(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest);

    /// @brief 释放处理Cosmos引擎发送通知时应答结果申请的内存
    /// @param callContext 发送者信息
    /// @param result 需要释放的内存块信息
    void Cosmos_ReleaseResult(const Cosmos_CallContext* callContext, const Cosmos_Result* result);

    /// @brief 处理Cosmos引擎发送的请求
    /// @param callContext 发送者信息
    /// @param invokeRequest 调用信息内容
    /// @return 调用处理结果
    Cosmos_InvokeResponse* Cosmos_Invoke(const Cosmos_CallContext* callContext, const Cosmos_InvokeRequest* invokeRequest);

    /// @brief 释放处理Cosmos引擎发送请求时应答结果申请的内存
    /// @param callContext 发送者信息
    /// @param invokeResponse 需要释放的内存块信息
    void Cosmos_ReleaseInvokeResponse(const Cosmos_CallContext* callContext, const Cosmos_InvokeResponse* invokeResponse);

    /// @brief 处理Cosmos引擎发送的订阅
    /// @param callContext 发送者信息
    /// @param subscribeRequest 订阅
    /// @return 订阅处理结果
    Cosmos_SubscribeResponse* Cosmos_Subscribe(const Cosmos_CallContext* callContext, const Cosmos_SubscribeRequest* subscribeRequest);

    /// @brief 处理Cosmos引擎发送的取消订阅
    /// @param callContext 发送者信息
    /// @param subscription 订阅信息
    /// @return 取消订阅处理结果
    Cosmos_Result* Cosmos_UnSubscribe(const Cosmos_CallContext* callContext, const Cosmos_Subscription* subscription);

	/// @brief 处理Cosmos引擎发送的订阅推送信息
    /// @param callContext 发送者信息
    /// @param subscribeRequest 推送内容
    /// @return 订阅推送处理结果
    Cosmos_Result* Cosmos_PushSubscriptionData(const Cosmos_CallContext* callContext, const Cosmos_SubscriptionDataFrame* subscriptionDataFrame);

    /// @brief 释放处理Cosmos引擎发送订阅时应答结果申请的内存
    /// @param callContext 发送者信息
    /// @param subscriptionResponse 需要释放的内存块信息
    void Cosmos_ReleaseSubscribeResponse(const Cosmos_CallContext* callContext, const Cosmos_SubscribeResponse* subscriptionResponse);


    //取消订阅消息
    void RegistNotify(notifyFunc func);

    //处理关闭
    void Close();

    //模拟订阅推送
    void SimulatePush();
private:
    CCosmosApi();
    ~CCosmosApi();
	std::string GetUUid();

    static CCosmosApi* m_instance;
    static std::mutex m_mutex;

private:
    Cosmos_EnvironmentCreationParameters* m_pEnvironment;           ///保存传给cosmos的初始化参数
    Cosmos_UninitializeEnvironmentDelegate m_pRelease;              ///释放cosmos环境

    Cosmos_NotifyDelegate m_pNotify;                                ///保存Cosmos引擎推送接口                   
    Cosmos_ReleaseResultDelegate m_pNotifyRelease;                  ///释放Cosmos引擎推送接口申请的内容

    Cosmos_InvokeDelegate m_pInvoke;                                ///保存Cosmos引擎调用接口
    Cosmos_ReleaseInvokeResponseDelegate m_pInvokeRelease;          ///释放Cosmos引擎调用接口申请的内容

    Cosmos_SubscribeDelegate m_pSubscribe;                          ///保存Cosmos引擎订阅信息接口
    Cosmos_ReleaseSubscribeResponseDelegate m_pSubscribeRelease;    ///释放Cosmos引擎订阅接口申请的内容

    Cosmos_UnsubscribeDelegate m_pUnsubscribe;                      ///保存Cosmos引擎取消订阅接口
    Cosmos_PushSubscriptionDataDelegate m_pPush;                    ///保存Cosmos引擎推送接口

    std::vector<notifyFunc>    m_vecNotifySub;
    std::map<std::string, std::vector<std::string>>  m_mapSubList;       ///保存订阅映射关系
    std::map<std::string, std::string>               m_mapIdToTopic;     ///保存id和topic的信息

    std::unique_ptr<platform::PeriodicTimer> m_timer;                    ///模拟订阅推送定时器（跨平台）
};
