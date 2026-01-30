
/*
* 文件名称：Cosmos.Product.Sdk.h 
* 文件用途：Cosmos 大核Sdk通用头文件，用于接入Cosmos
* 联系作者：wangkai2@myhexin.com, gaoqifei@myhexin.com
* 命名规范：
*   1. 类型命名，使用唯一下划线来隔离命名空间和类型名
*   2. 其余命名遵循dotnet coding convention
* 使用说明：
*   1. CosmosApi标注的为可调用Api
*/

#pragma once

#pragma region 编译预处理

#ifdef _WIN32
    #ifdef _WINDLL
        #ifdef COSMOS_API_EXPORT
            #define CosmosApi __declspec(dllexport)
        #else 
            #define CosmosApi __declspec(dllimport)
        #endif
    #else
        #define CosmosApi
    #endif
#elif  __linux__
    #ifdef __GNUC__
        #if __GNUC__ >= 4
            #define CosmosApi __attribute__ ((visibility("default")))
        #else
            #define CosmosApi
        #endif
    #else
        #error "Cosmos sdk supports gcc only on linux."
    #endif
#elif __APPLE__
    #ifdef __clang__
        #define CosmosApi
    #else
        #error "Cosmos sdk supports clang only on MacOS."
    #endif
#else
    #error "Cosmos cannot support current OS."
#endif

#ifdef __cplusplus
    #include <cstdint>
    extern "C"
    {
#else
    #include <stdint.h> 
#endif

#pragma endregion

#pragma region 0. 系统对象定义

    /// <summary>
    /// Cosmos 操作系统资源句柄，注意，本句柄由指针型重定义，即x86下4字节，x64和arm64下8字节
    /// </summary>
    typedef void* Cosmos_Handle;

    /// <summary>
    /// Cosmos 窗体句柄
    /// </summary>
    typedef Cosmos_Handle Cosmos_WindowHandle;

#pragma endregion

#pragma region 1. Gui结构定义

    /// <summary>
    /// 可视化元素可见性
    /// </summary>
    typedef enum _Cosmos_GuiVisibility
    {
        Show,       // 可见（正常显示）
        Hidden,     // 隐藏（占位，占区大小不变）
        Collapsed   // 坍缩（不占位，占区大小归零）
    } Cosmos_GuiVisibility;

    /// <summary>
    /// 可视化HighDPI模式
    /// </summary>
    typedef enum _Comos_GuiHighDpiMode
    {
        DpiUnaware          = 0, // 应用程序窗口不会随着 DPI 更改而缩放，始终假定缩放比例为 100 % 。
        SystemAware         = 1, // 此窗口会查询一次主监视器的 DPI，并将其用于所有监视器上的应用程序。
        PerMonitor          = 2, // 此窗口会在创建 DPI 时对其进行检查，并在 DPI 更改时调整缩放比例。
        PerMonitorV2        = 3, // 类似于 PerMonitor，但启用了子窗口 DPI 更改通知、comctl32 控件的改进缩放和对话框缩放。
        DpiUnawareGdiScaled = 4, // 类似于 DpiUnaware，但提高了基于 GDI / GDI + 的内容的质量。
    } Comos_GuiHighDpiMode;

#pragma endregion

#pragma region 2. 数据结构定义

    /// <summary>
    /// Cosmos 对象句柄
    /// </summary>
    typedef Cosmos_Handle Cosmos_ObjectHandle;

    /// <summary>
    /// Cosmos结果状态结构
    /// </summary>
    typedef struct _CosmosResult
    {
        /// <summary>
        /// 结果代码
        /// </summary>
        int64_t Code;

        /// <summary>
        /// 结果消息
        /// </summary>
        const char* Message;

    } Cosmos_Result;

    /// <summary>
    /// Cosmos数据帧结构
    /// </summary>
    typedef struct _Cosmos_DataFrame
    {
        /// <summary>
        /// 数据
        /// </summary>
        const char* Data;

        /// <summary>
        /// 数据大小
        /// </summary>
        int32_t DataSize;

    } Cosmos_DataFrame;

#pragma endregion

#pragma region 3. 通信结构定义

    /// <summary>
    /// 调用上下文
    /// </summary>
    typedef struct _Cosmos_CallContext
    {
        /// <summary>
        /// 调用者对象句柄
        /// </summary>
        Cosmos_Handle CallerHandle;

        /// <summary>
        /// 被调者对象句柄
        /// </summary>
        Cosmos_Handle TargetHandle;

    } Cosmos_CallContext;

    /// <summary>
    /// 通知请求
    /// </summary>
    typedef struct _Cosmos_NotifyRequest
    {
        /// <summary>
        /// 通知主题
        /// </summary>
        const char* Topic;

        /// <summary>
        /// 通知标识
        /// </summary>
        const char* RoutingKey;

        /// <summary>
        /// 消息
        /// </summary>
        const char* Message;

    } Cosmos_NotifyRequest;

    /// <summary>
    /// 调用请求
    /// </summary>
    typedef struct _Cosmos_InvokeRequest
    {
        /// <summary>
        /// 调用方法
        /// </summary>
        const char* Method;

        /// <summary>
        /// 调用参数
        /// </summary>
        const char* Parameters;

    } Cosmos_InvokeRequest;

    /// <summary>
    /// 调用返回
    /// </summary>
    typedef struct _Cosmos_InvokeResponse
    {
        /// <summary>
        /// 调用结果
        /// </summary>
        const Cosmos_Result* Result;

        /// <summary>
        /// 返回值
        /// </summary>
        const Cosmos_DataFrame* DataFrame;

    } Cosmos_InvokeResponse;

    /// <summary>
    /// 推送数据帧
    /// </summary>
    typedef struct _Cosmos_SubscriptionDataFrame
    {
        /// <summary>
        /// 订阅Id
        /// </summary>
        const char* SubscriptionId;

        /// <summary>
        /// 数据帧
        /// </summary>
        const Cosmos_DataFrame* DataFrame;

    } Cosmos_SubscriptionDataFrame;

    /// <summary>
    /// 订阅结构
    /// </summary>
    typedef struct _Cosmos_SubscribeRequest
    {
        /// <summary>
        /// 订阅主题
        /// </summary>
        const char* Topic;

        /// <summary>
        /// 订阅参数
        /// </summary>
        const char* Parameters;

    } Cosmos_SubscribeRequest;

    /// <summary>
    /// 订阅结构
    /// </summary>
    typedef struct _CosmosSubscription
    {
        /// <summary>
        /// 订阅Id
        /// </summary>
        const char* SubscriptionId;

    } Cosmos_Subscription;
    

    /// <summary>
    /// 订阅响应
    /// </summary>
    typedef struct _Cosmos_SubscribeResponse
    {
        /// <summary>
        /// 订阅结果（该Result无需单独释放）
        /// </summary>
        const Cosmos_Result* Result;

        /// <summary>
        /// 订阅
        /// </summary>
        const Cosmos_Subscription* Subscription;

    } Cosmos_SubscribeResponse;

#pragma endregion

#pragma region 4. 通信接口委托定义

    /// <summary>
    /// 函数类型：通知
    /// </summary>
    typedef Cosmos_Result* (*Cosmos_NotifyDelegate)(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest);

    /// <summary>
    /// 函数类型：调用
    /// </summary>
    typedef Cosmos_InvokeResponse* (*Cosmos_InvokeDelegate)(const Cosmos_CallContext* callContext, const Cosmos_InvokeRequest* invokeRequest);

    /// <summary>
    /// 函数类型：释放调用返回值对象
    /// </summary>
    typedef void (*Cosmos_ReleaseInvokeResponseDelegate)(const Cosmos_CallContext* callContext, const Cosmos_InvokeResponse* invokeResponse);

    /// <summary>
    /// 函数类型：订阅
    /// </summary>
    typedef Cosmos_SubscribeResponse* (*Cosmos_SubscribeDelegate)(const Cosmos_CallContext* callContext, const Cosmos_SubscribeRequest* subscribeRequest);

    /// <summary>
    /// 函数类型：释放订阅返回值对象
    /// </summary>
    typedef void (*Cosmos_ReleaseSubscribeResponseDelegate)(const Cosmos_CallContext* callContext, const Cosmos_SubscribeResponse* subscriptionResponse);

    /// <summary>
    /// 函数类型：推送订阅数据帧
    /// </summary>
    typedef Cosmos_Result* (*Cosmos_PushSubscriptionDataDelegate) (const Cosmos_CallContext* callContext, const Cosmos_SubscriptionDataFrame* subscriptionDataFrame);

    /// <summary>
    /// 函数类型：取消订阅
    /// </summary>
    typedef Cosmos_Result* (*Cosmos_UnsubscribeDelegate) (const Cosmos_CallContext* callContext, const Cosmos_Subscription* subscription);

    /// <summary>
    /// 函数类型：释放结果
    /// </summary>
    typedef void (*Cosmos_ReleaseResultDelegate) (const Cosmos_CallContext* callContext, const Cosmos_Result* result);

#pragma endregion

#pragma region 5. 大核职责结构

    /// <summary>
    /// 大核职责结构（由大核实现，小核调用）
    /// </summary>
    typedef struct _Cosmos_Responsibility
    {
        /// <summary>
        /// 外部通知处理
        /// </summary>
        Cosmos_NotifyDelegate Cosmos_NotifyHandler;

        /// <summary>
        /// 外部调用处理
        /// </summary>
        Cosmos_InvokeDelegate Cosmos_InvokeHandler;

        /// <summary>
        /// 外部释放调用返回值处理
        /// </summary>
        Cosmos_ReleaseInvokeResponseDelegate Cosmos_ReleaseInvokeResponseHandler;

        /// <summary>
        /// 外部订阅处理
        /// </summary>
        Cosmos_SubscribeDelegate Cosmos_SubscribeHandler;

        /// <summary>
        /// 外部释放订阅结构
        /// </summary>
        Cosmos_ReleaseSubscribeResponseDelegate Cosmos_ReleaseSubscribeResponseHandler;

        /// <summary>
        /// 外部推送处理
        /// </summary>
        Cosmos_PushSubscriptionDataDelegate Cosmos_PushSubscriptionDataHandler;

        /// <summary>
        /// 外部取消订阅处理
        /// </summary>
        Cosmos_UnsubscribeDelegate Cosmos_UnsubscribeHandler;

        /// <summary>
        /// 外部释放结果处理
        /// </summary>
        Cosmos_ReleaseResultDelegate Cosmos_ReleaseResultHandler;

    } Cosmos_Responsibility;

#pragma endregion

#pragma region 6. 环境管理结构

    /// <summary>
    /// 国际化参数
    /// </summary>
    typedef struct _Cosmos_GlobalizationParameters
    {
        /// <summary>
        /// 区域Id，如cn/us等
        /// </summary>
        const char* RegionId;

        /// <summary>
        /// 语言Id，如zh-cn/en-us等
        /// </summary>
        const char* LanguageId;

        /// <summary>
        /// 时区id，如e8/w5等
        /// </summary>
        const char* TimeZoneId;

        /// <summary>
        /// 货币类型
        /// </summary>
        const char* Currency;
    } Cosmos_GlobalizationParameters;

    /// <summary>
    /// 客户端参数
    /// </summary>
    typedef struct _Cosmos_ClientParameters
    {
        /// <summary>
        /// 客户端Id：如iFinD，tyb等
        /// </summary>
        const char* Id;

        /// <summary>
        /// 客户端Sku：如B2C版，机构版，某券商版，苹果版等
        /// </summary>
        const char* Sku;

        /// <summary>
        /// 版本号形式必须是"{d}.{d}.{d}.{d}"形式，1~4位均可，即
        /// </summary>
        const char* Version;

        /// <summary>
        /// Cosmos主程序文件路径
        /// </summary>
        const char* CosmosMainAppPath;

        /// <summary>
        /// DPI适应模式（传0或1就好）
        /// </summary>
        Comos_GuiHighDpiMode HighDpiMode;

    } Cosmos_ClientParameters;

    /// <summary>
    /// 代理参数
    /// </summary>
    typedef struct _Cosmos_ProxyParameters
    {
        void* todo;
        // todo
    } Cosmos_ProxyParameters;

    /// <summary>
    /// 视觉参数
    /// </summary>
    typedef struct _Cosmos_VisualParameters
    {
        /// <summary>
        /// Gui主题框架 "fluent" / "cosmos" / "ifind" / "tyb"
        /// </summary>
        const char* GuiThemeFrameworks;

        /// <summary>
        /// Gui主题色系
        /// </summary>
        const char* GuiThemeColorScheme;

    } Cosmos_VisualParameters;

    /// <summary>
    /// 开发者参数
    /// </summary>
    typedef struct _Cosmos_DeveloperParameter
    {
        /// <summary>
        /// 运行模式 (Debug / Release)
        /// </summary>
        const char* RuntimeMode;

        /// <summary>
        /// 默认Gui管理窗显示模式 Show/Hide
        /// </summary>
        const char* GuiMode;

        /// <summary>
        /// Cosmos主程序地址
        /// </summary>
        const char* CosmosExecutiveFile;

        /// <summary>
        /// App提供者模式 Nuget/tsmall/local
        /// </summary>
        const char* AppProviderMode;

        /// <summary>
        /// App提供者url
        /// </summary>
        const char* AppProviderUrl;

        /// <summary>
        /// 环境创建超时时间(毫秒）
        /// </summary>
        int32_t Timeout;

    } Cosmos_DeveloperParameter;

    /// <summary>
    /// 浏览器参数
    /// </summary>
    typedef struct _Cosmos_WebViewParameters
    {
        /// <summary>
        /// Cef根目录
        /// </summary>
        const char* CefDirectory;

        /// <summary>
        /// Cef资源目录
        /// </summary>
        const char* CefResourcesDirectory;

        /// <summary>
        /// Cef语言目录
        /// </summary>
        const char* CefLocaleDirectory;

        /// <summary>
        /// Cef额外参数（各大核根据自身情况配置，如--gpu-acceleration之类）
        /// </summary>
        const char* CefExtraParameters;
    } Cosmos_WebViewParameters;


    /// <summary>
    /// Cosmos环境创建参数
    /// </summary>
    typedef struct _Cosmos_EnvironmentCreationParameters
    {
        /// <summary>
        /// 客户端参数（设置后只读）
        /// </summary>
        Cosmos_ClientParameters* ClientParameters;

        /// <summary>
        /// 代理参数（设置后只读）
        /// </summary>
        Cosmos_ProxyParameters* ProxyParameters;

        /// <summary>
        /// 浏览器参数（设置后只读）
        /// </summary>
        Cosmos_WebViewParameters* WebViewParameters;

        /// <summary>
        /// 开发者参数（设置后只读）
        /// </summary>
        Cosmos_DeveloperParameter* DeveloperParameter;

        /// <summary>
        /// 全球化参数（可随时读写）
        /// </summary>
        Cosmos_GlobalizationParameters* GlobalizationParameters;

        /// <summary>
        /// 视觉参数（可随时读写）
        /// </summary>
        Cosmos_VisualParameters* VisualParameters;

        /// <summary>
        /// 大核承担的职责（由大核负责注入）
        /// </summary>
        Cosmos_Responsibility* Responsibility;

    } Cosmos_EnvironmentCreationParameters;

    /// <summary>
    /// 环境句柄
    /// </summary>
    typedef Cosmos_Handle Cosmos_EvironmentHandle;

    /// <summary>
    /// 环境事件参数
    /// </summary>
    typedef struct _Cosmos_EnvironmentEventArgs
    {
        /// <summary>
        /// 事件名
        /// </summary>
        const char* EventName;

        /// <summary>
        /// 事件上下文字符串
        /// </summary>
        const char* EventContext;

    } Cosmos_EnvironmentEventArgs;


#pragma endregion

#pragma region 7. 会话管理结构

    /// <summary>
    /// 会话用 - 账户参数
    /// </summary>
    typedef struct _Cosmos_Account
    {
        const char* AccountId;
        // todo 
    } Cosmos_AccountParameters;

    /// <summary>
    /// 会话用 - Cookie参数
    /// </summary>
    typedef struct _Cosmos_CookieParameters
    {
        void* todo;
    } Cosmos_CookieParameters;

    /// <summary>
    /// 会话句柄
    /// </summary>
    typedef Cosmos_Handle Cosmos_SessionHandle;

    /// <summary>
    /// 会话创建参数
    /// </summary>
    typedef struct _Cosmos_SessionCreationParameters
    {
        /// <summary>
        /// 账户参数
        /// </summary>
        Cosmos_AccountParameters* AccountParameters;

        /// <summary>
        /// Cookie参数
        /// </summary>
        Cosmos_CookieParameters* CookieParameters;
    } Cosmos_SessionCreationParameters;

    /// <summary>
    /// 会话对象
    /// </summary>
    typedef struct _Cosmos_Session
    {
        /// <summary>
        /// 大核句柄
        /// </summary>
        Cosmos_Handle ClientHandle;

        /// <summary>
        /// 小核句柄
        /// </summary>
        Cosmos_Handle ServerHandle;

    } Cosmos_Session;

    /// <summary>
    /// 创建会话结果
    /// </summary>
    typedef struct _Cosmos_SessionCreationResult
    {
        Cosmos_Result* Result;
        Cosmos_Session* Session;
    } Cosmos_SessionCreationResult;

#pragma endregion

#pragma region 8. 通用通信接口

    /// <summary>
    /// 会话事件处理，注意接入方无需销毁该参数
    /// </summary>
    typedef void (*Cosmos_EnvironmentEventHandler)(Cosmos_EnvironmentEventArgs* environmentEventArgs);

    /// <summary>
    /// CosmosApi - 初始化环境
    /// </summary>
    /// <param name="environmentParameters"></param>
    /// <param name="evironmentEventHandler"></param>
    /// <returns></returns> 
    CosmosApi Cosmos_Result* Cosmos_InitializeEnvironment(const Cosmos_CallContext* callContext, const Cosmos_EnvironmentCreationParameters* environmentCreationParameters, Cosmos_EnvironmentEventHandler evironmentEventHandler);
    typedef Cosmos_Result* (*Cosmos_InitializeEnvironmentDelegate)(const Cosmos_CallContext* callContext, const Cosmos_EnvironmentCreationParameters* environmentCreationParameters, Cosmos_EnvironmentEventHandler evironmentEventHandler);

    /// <summary>
    /// CosmosApi - 终结化环境
    /// </summary>
    /// <param name="environment"></param>
    /// <returns></returns>
    CosmosApi Cosmos_Result* Cosmos_UninitializeEnvironment();
    typedef Cosmos_Result* (*Cosmos_UninitializeEnvironmentDelegate) ();

    /// <summary>
    /// CosmosApi - 创建会话
    /// </summary>
    /// <returns></returns>
    CosmosApi Cosmos_SessionCreationResult* Cosmos_CreateSession(const Cosmos_CallContext* callContext, const Cosmos_SessionCreationParameters* sessionCreationParameters);
    typedef Cosmos_SessionCreationResult* (*Cosmos_CreateSessionDelegate)(const Cosmos_CallContext* callContext, const Cosmos_SessionCreationParameters* sessionCreationParameters);

    /// <summary>
    /// CosmosApi - 销毁会话
    /// </summary>
    /// <returns></returns>
    CosmosApi Cosmos_Result* Cosmos_DestroySession(const Cosmos_CallContext* callContext, Cosmos_SessionCreationResult* sessionCreationResult);
    typedef Cosmos_Result* (*Cosmos_DestroySessionDelegate)(const Cosmos_CallContext* callContext, Cosmos_SessionCreationResult* sessionCreationResult);

    /// <summary>
    /// CosmosApi - 通知，对应类型：Cosmos_NotifyDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="notifyRequest"></param>
    /// <returns></returns>
    CosmosApi Cosmos_Result* Cosmos_Notify(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest);

    /// <summary>
    /// CosmosApi - 调用，对应类型：Cosmos_InvokeDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="invokeRequest"></param>
    /// <returns></returns>
    CosmosApi Cosmos_InvokeResponse* Cosmos_Invoke(const Cosmos_CallContext* callContext, const Cosmos_InvokeRequest* invokeRequest);

    /// <summary>
    /// CosmosApi - 释放调用响应，对应类型：Cosmos_ReleaseInvokeResponseDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="invokeResponse"></param>
    /// <returns></returns>
    CosmosApi void Cosmos_ReleaseInvokeResponse(const Cosmos_CallContext* callContext, const Cosmos_InvokeResponse* invokeResponse);

    /// <summary>
    /// CosmosApi - 订阅，对应类型：Cosmos_SubscribeDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="subscribeRequest"></param>
    /// <returns></returns>
    CosmosApi Cosmos_SubscribeResponse* Cosmos_Subscribe(const Cosmos_CallContext* callContext, const Cosmos_SubscribeRequest* subscribeRequest);

    /// <summary>
    /// CosmosApi - 释放订阅响应，对应类型：Cosmos_ReleaseSubscribeResponseDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="subscriptionResponse"></param>
    /// <returns></returns>
    CosmosApi void Cosmos_ReleaseSubscribeResponse(const Cosmos_CallContext* callContext, const Cosmos_SubscribeResponse* subscriptionResponse);

    /// <summary>
    /// CosmosApi - 推送订阅数据，对应类型：Cosmos_PushSubscriptionDataDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="subscriptionDataFrame"></param>
    /// <returns></returns>
    CosmosApi Cosmos_Result* Cosmos_PushSubscriptionData(const Cosmos_CallContext* callContext, const Cosmos_SubscriptionDataFrame* subscriptionDataFrame);

    /// <summary>
    /// CosmosApi - 解订阅，对应类型：Cosmos_UnsubscribeDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="subscription"></param>
    /// <returns></returns>
    CosmosApi Cosmos_Result* Cosmos_Unsubscribe(const Cosmos_CallContext* callContext, const Cosmos_Subscription* subscription);

    /// <summary>
    /// CosmosApi - 释放结果，对应类型：Cosmos_ReleaseResultDelegate
    /// </summary>
    /// <param name="callContext"></param>
    /// <param name="result"></param>
    /// <returns></returns>
    CosmosApi void Cosmos_ReleaseResult(const Cosmos_CallContext* callContext, const Cosmos_Result* result);

#pragma endregion

#pragma region 9. App业务接口

    /// <summary>
    /// AppGui容器
    /// </summary>
    typedef struct _Cosmos_AppGuiContainer
    {
        /// <summary>
        /// 窗口句柄（与Container一定是一对一关系）
        /// </summary>
        Cosmos_WindowHandle WindowHandle;

        /// <summary>
        /// 所有Widget句柄数组（每个Container中会有多个Widget）
        /// </summary>
        Cosmos_ObjectHandle* WidgetHandles;

        /// <summary>
        /// Widget句柄数量
        /// </summary>
        int64_t WidgetHandlesCount;

    } Cosmos_AppGuiContainer;

    /// <summary>
    /// AppGui容器属性
    /// </summary>
    typedef struct _Cosmos_AppGuiContainerParameters
    {
        /// <summary>
        /// AppGuid
        /// </summary>
        const char* appGuid;

        /// <summary>
        /// Widget Guid
        /// </summary>
        const char* widgetGuid;

        /// <summary>
        /// 窗口可见性
        /// </summary>
        Cosmos_GuiVisibility WindowVisibility;

        /// <summary>
        /// 窗口标题栏可见性
        /// </summary>
        Cosmos_GuiVisibility WindowTitleBarVisibility;

        /// <summary>
        /// 边框可见性
        /// </summary>
        Cosmos_GuiVisibility WindowBorderVisibility;

    } Cosmos_AppGuiContainerParameters;
    
    /// <summary>
    /// 创建AppGui容器
    /// </summary>
    /// <param name="appGuid"></param>
    /// <param name="widgetGuid"></param>
    /// <param name="guiContainerProperties"></param>
    /// <returns></returns>
    CosmosApi Cosmos_AppGuiContainer* Cosmos_CreateAppGuiContainer(const Cosmos_CallContext* callContext, const Cosmos_AppGuiContainerParameters* appGuiContainerParameters);

    /// <summary>
    /// 虚拟应用调用参数
    /// </summary>
    typedef struct _Cosmos_VirtualAppParameters
    {
        /// <summary>
        /// 虚拟应用Guid
        /// </summary>
        char* virtualAppGuid;

        /// <summary>
        /// 虚拟方法Guid
        /// </summary>
        char* virtualAppMethodGuid;

        /// <summary>
        /// 虚拟调用参数
        /// </summary>
        char* parameters;
    } Cosmos_VirtualAppParameters;

    /// <summary>
    /// 运行虚拟应用方法
    /// </summary>
    /// <param name="virtualAppGuid"></param>
    /// <param name="virtualAppMethodGuid"></param>
    /// <param name="parameters"></param>
    /// <returns></returns>
    CosmosApi Cosmos_Result* Cosmos_InvokeVirtualAppMethod(const Cosmos_CallContext* callContext, const Cosmos_VirtualAppParameters* virtualAppParameters);

    typedef Cosmos_Result* (*Cosmos_InvokeVirtualAppMethodDelegate)(const Cosmos_CallContext* callContext, const Cosmos_VirtualAppParameters* virtualAppParameters);

    /// <summary>
    /// 注册虚拟应用回调方法
    /// </summary>
    /// <param name="virtualAppMethodHandler"></param>
    /// <returns></returns>
    CosmosApi void Cosmos_SetVirtualAppMethodHandler(Cosmos_InvokeVirtualAppMethodDelegate virtualAppMethodHandler);


    /// <summary>
    /// Cosmos对象状态
    /// </summary>
    typedef struct _Cosmos_ObjectStatus
    {
        /// <summary>
        /// 数据
        /// </summary>
        const Cosmos_DataFrame* DataFrame;
        
    } Cosmos_ObjectStatus;

    /// <summary>
    /// 销毁App布局容器
    /// </summary>
    /// <param name="containerHandle"></param>
    /// <returns></returns>
    CosmosApi Cosmos_Result* CosmosApp_DestroyGuiContainer(const Cosmos_CallContext* callContext, Cosmos_AppGuiContainer* appGuiContainer);

    /// <summary>
    /// 导出目标对象状态
    /// </summary>
    /// <param name="objectHandle"></param>
    /// <returns></returns>
    CosmosApi Cosmos_ObjectStatus* CosmosApp_ExportTargetStatus(const Cosmos_CallContext* callContext);

    /// <summary>
    /// 导入目标对象状态
    /// </summary>
    /// <param name="targetHandle"></param>
    /// <param name="status"></param>
    /// <returns></returns>
    CosmosApi void CosmosApp_ImportTargetStatus(const Cosmos_CallContext* callContext, Cosmos_ObjectStatus* objectStatus);

    /// <summary>
    /// 导出目标对象默认状态
    /// </summary>
    /// <param name="targetHandle"></param>
    /// <returns></returns>
    CosmosApi Cosmos_ObjectStatus* CosmosApp_ExportTargetDefaultStatus(const Cosmos_CallContext* callContext);

    /// <summary>
    /// 导入目标对象默认状态
    /// </summary>
    /// <param name="targetHandle"></param>
    /// <returns></returns>
    CosmosApi void CosmosApp_ImportTargetDefaultStatus(const Cosmos_CallContext* callContext);

    /// <summary>
    /// 释放status返回值
    /// </summary>
    /// <param name="status"></param>
    /// <returns></returns>
    CosmosApi void CosmosApp_ReleaseTargetStatus(const Cosmos_CallContext* callContext, Cosmos_ObjectStatus* objectStatus);

#pragma endregion
 
#pragma region 编译尾处理

#ifdef __cplusplus
}
#endif

#pragma endregion
