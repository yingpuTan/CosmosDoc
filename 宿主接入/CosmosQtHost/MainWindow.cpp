
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QWindow>
#include <QCloseEvent>
#include <QDebug>

#include "Cosmos.Product.Sdk.h"
#include "platform.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

// ---------------- Cosmos C SDK 直接封装（最小化） ----------------

namespace {

platform::DynamicLibrary g_sdkModule;
Cosmos_EnvironmentCreationParameters* g_envParams = nullptr;
Cosmos_InitializeEnvironmentDelegate g_initEnv = nullptr;
Cosmos_UninitializeEnvironmentDelegate g_uninitEnv = nullptr;
Cosmos_InvokeDelegate g_invoke = nullptr;
Cosmos_ReleaseInvokeResponseDelegate g_releaseInvoke = nullptr;

// Base64 编码（复制自 CosmosHost 示例）
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string base64_encode(const std::string& in)
{
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

inline std::string gbk_to_utf8(const std::string& s) { return platform::gbk_to_utf8(s); }
inline std::string utf8_to_gbk(const std::string& s) { return platform::utf8_to_gbk(s); }

std::string GetSdkLibraryName()
{
#if defined(_WIN32)
    return "CosmosSDK.dll";
#else
    return "libCosmosSDK.so";
#endif
}

std::string GetMainAppName()
{
#if defined(_WIN32)
    return "Cosmos.MainApp.exe";
#else
    return "Cosmos.MainApp.CrossPlatform";
#endif
}

// 宿主回调：本 Demo 只简单返回成功，方便引擎初始化
Cosmos_Result* Cosmos_Notify_Callback(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest)
{
    Q_UNUSED(callContext);
    Q_UNUSED(notifyRequest);
    Cosmos_Result* result = new Cosmos_Result;
    result->Code = 200;
    result->Message = "";
    return result;
}

void Cosmos_ReleaseResult_Callback(const Cosmos_CallContext* callContext, const Cosmos_Result* result)
{
    Q_UNUSED(callContext);
    delete result;
}

Cosmos_InvokeResponse* Cosmos_Invoke_Callback(const Cosmos_CallContext* callContext, const Cosmos_InvokeRequest* invokeRequest)
{
    Q_UNUSED(callContext);
    Q_UNUSED(invokeRequest);
    // 本 Demo 不处理来自 Cosmos 的反向 Invoke，返回一个“未实现”
    Cosmos_Result* r = new Cosmos_Result;
    r->Code = 501;
    r->Message = "not implemented";
    Cosmos_DataFrame* df = new Cosmos_DataFrame;
    df->Data = "";
    df->DataSize = 0;
    Cosmos_InvokeResponse* resp = new Cosmos_InvokeResponse;
    resp->Result = r;
    resp->DataFrame = df;
    return resp;
}

void Cosmos_ReleaseInvokeResponse_Callback(const Cosmos_CallContext* callContext, const Cosmos_InvokeResponse* invokeResponse)
{
    Q_UNUSED(callContext);
    if (!invokeResponse) return;
    delete invokeResponse->Result;
    delete invokeResponse->DataFrame;
    delete invokeResponse;
}

Cosmos_SubscribeResponse* Cosmos_Subscribe_Callback(const Cosmos_CallContext* callContext, const Cosmos_SubscribeRequest* subscribeRequest)
{
    Q_UNUSED(callContext);
    Q_UNUSED(subscribeRequest);
    Cosmos_Result* r = new Cosmos_Result;
    r->Code = 200;
    r->Message = "";
    Cosmos_Subscription* sub = new Cosmos_Subscription;
    sub->SubscriptionId = "";
    Cosmos_SubscribeResponse* resp = new Cosmos_SubscribeResponse;
    resp->Result = r;
    resp->Subscription = sub;
    return resp;
}

Cosmos_Result* Cosmos_UnSubscribe_Callback(const Cosmos_CallContext* callContext, const Cosmos_Subscription* subscription)
{
    Q_UNUSED(callContext);
    Q_UNUSED(subscription);
    Cosmos_Result* r = new Cosmos_Result;
    r->Code = 200;
    r->Message = "";
    return r;
}

Cosmos_Result* Cosmos_PushSubscriptionData_Callback(const Cosmos_CallContext* callContext, const Cosmos_SubscriptionDataFrame* subscriptionDataFrame)
{
    Q_UNUSED(callContext);
    Q_UNUSED(subscriptionDataFrame);
    Cosmos_Result* r = new Cosmos_Result;
    r->Code = 200;
    r->Message = "";
    return r;
}

void Cosmos_ReleaseSubscribeResponse_Callback(const Cosmos_CallContext* callContext, const Cosmos_SubscribeResponse* subscriptionResponse)
{
    Q_UNUSED(callContext);
    if (!subscriptionResponse) return;
    delete subscriptionResponse->Result;
    delete subscriptionResponse->Subscription;
    delete subscriptionResponse;
}

} // namespace

// ---------------- MainWindow ----------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    auto *layout  = new QVBoxLayout(central);

    auto *btnCreate = new QPushButton(QStringLiteral("创建并嵌入 Cosmos 组件"), this);
    m_cosmosContainer = new QWidget(this);
    m_cosmosContainer->setMinimumSize(600, 400);
    m_cosmosContainer->setStyleSheet(QStringLiteral("background-color:#202020;"));

    layout->addWidget(btnCreate);
    layout->addWidget(m_cosmosContainer, 1);
    setCentralWidget(central);

    connect(btnCreate, &QPushButton::clicked,
            this, &MainWindow::onCreateWidgetClicked);

    setWindowTitle(QStringLiteral("Cosmos Qt 宿主 Demo"));
    resize(1000, 700);

    initCosmos();
}

MainWindow::~MainWindow()
{
    // 清理嵌入的窗口
    // 注意：QWidget::createWindowContainer 创建的容器会管理 QWindow 的生命周期
    // 删除 m_embeddedWidget 时，Qt 会自动删除它关联的 m_embeddedWindow
    if (m_embeddedWidget) {
        m_embeddedWidget->setParent(nullptr);
        delete m_embeddedWidget;
        m_embeddedWidget = nullptr;
        // m_embeddedWindow 已被自动删除，设置为 nullptr 避免悬空指针
        m_embeddedWindow = nullptr;
    } else if (m_embeddedWindow) {
        // 如果 m_embeddedWidget 不存在但 m_embeddedWindow 存在（createWindowContainer 失败的情况）
        // 需要手动删除 m_embeddedWindow
        delete m_embeddedWindow;
        m_embeddedWindow = nullptr;
    }
    
    if (g_uninitEnv) {
        g_uninitEnv();
    }
    if (g_envParams) {
        delete g_envParams->Responsibility;
        delete g_envParams->ClientParameters;
        delete g_envParams->DeveloperParameter;
        delete g_envParams->WebViewParameters;
        delete g_envParams;
        g_envParams = nullptr;
    }
    g_sdkModule.unload();
}

void MainWindow::onCreateWidgetClicked()
{
    createAndEmbedWidget();
}

void MainWindow::initCosmos()
{
    // 1. 动态加载 CosmosSDK
    std::string exeDir = platform::executable_dir();
    std::string sdkPath = platform::path_join(platform::path_join(exeDir, "Cosmos"), GetSdkLibraryName());
    if (!g_sdkModule.load(sdkPath)) {
        qDebug() << "Failed to load Cosmos SDK:" << QString::fromStdString(g_sdkModule.last_error());
        return;
    }

    // 2. 取出 C API 函数指针
    g_initEnv = reinterpret_cast<Cosmos_InitializeEnvironmentDelegate>(g_sdkModule.symbol("Cosmos_InitializeEnvironment"));
    g_uninitEnv = reinterpret_cast<Cosmos_UninitializeEnvironmentDelegate>(g_sdkModule.symbol("Cosmos_UninitializeEnvironment"));
    g_invoke = reinterpret_cast<Cosmos_InvokeDelegate>(g_sdkModule.symbol("Cosmos_Invoke"));
    g_releaseInvoke = reinterpret_cast<Cosmos_ReleaseInvokeResponseDelegate>(g_sdkModule.symbol("Cosmos_ReleaseInvokeResponse"));

    if (!g_initEnv || !g_uninitEnv || !g_invoke || !g_releaseInvoke) {
        qDebug() << "Failed to get Cosmos C API function pointers";
        return;
    }

    // 3. 组装环境参数（基本照抄 CosmosHost 示例）
    Cosmos_ClientParameters* clientParameters = new Cosmos_ClientParameters;
    memset(clientParameters, 0, sizeof(Cosmos_ClientParameters));

    static std::string mainAppPath = platform::path_join("./Cosmos", GetMainAppName());
    clientParameters->CosmosMainAppPath = const_cast<char*>(mainAppPath.c_str());

    // 产品&行情信息，照示例构造并 Base64
    rapidjson::Document docProduct, docMarketAccount;
    std::string dataProduct, dataMarketAccount;
    {
        docProduct.SetObject();
        docProduct.AddMember(rapidjson::StringRef("Account"), "22282", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Token"), "", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Password"), "123123", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("ProductID"), "GMatrix", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("SpiderUrl"), "https://unitetest.chinastock.com.cn:8081", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Ip"), "unitetest.chinastock.com.cn", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Port"), 9999, docProduct.GetAllocator());

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w(buf);
        docProduct.Accept(w);
        dataProduct = gbk_to_utf8(buf.GetString());
        dataProduct = base64_encode(dataProduct);
    }
    {
        docMarketAccount.SetObject();
        docMarketAccount.AddMember(rapidjson::StringRef("Account"), "test", docProduct.GetAllocator());
        docMarketAccount.AddMember(rapidjson::StringRef("Md5"), "123", docProduct.GetAllocator());

        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> w(buf);
        docMarketAccount.Accept(w);
        dataMarketAccount = gbk_to_utf8(buf.GetString());
        dataMarketAccount = base64_encode(dataMarketAccount);
    }

    // 使用字符数组避免反引号在字符串字面量中的潜在问题
    static const char separator[] = "```";
    static std::string idStr = std::string("HostDemo-") + separator + dataMarketAccount + separator + dataProduct + separator;
    clientParameters->Id = idStr.c_str();
    clientParameters->Version = "0.0.0.1";
    clientParameters->HighDpiMode = Comos_GuiHighDpiMode::SystemAware;

    Cosmos_DeveloperParameter* developerParameters = new Cosmos_DeveloperParameter;
    memset(developerParameters, 0, sizeof(Cosmos_DeveloperParameter));
    developerParameters->AppProviderMode = "nuget;https://unitetest.chinastock.com.cn:453/v3/index.json";
    //developerParameters->AppProviderMode = "local";
    developerParameters->RuntimeMode = "debug";
    developerParameters->GuiMode = "show";

    Cosmos_WebViewParameters* webViewParameters = new Cosmos_WebViewParameters;
    memset(webViewParameters, 0, sizeof(Cosmos_WebViewParameters));
    std::string cefPath = platform::path_join(platform::path_join(exeDir, "Cosmos"), "cef");
    std::string cefResourcesPath = platform::path_join(cefPath, "Resources");
    std::string cefLocalPath = platform::path_join(cefResourcesPath, "locales");
    webViewParameters->CefDirectory = cefPath.c_str();
    webViewParameters->CefResourcesDirectory = cefResourcesPath.c_str();
    webViewParameters->CefLocaleDirectory = cefLocalPath.c_str();

    Cosmos_Responsibility* resp = new Cosmos_Responsibility;
    memset(resp, 0, sizeof(Cosmos_Responsibility));
    resp->Cosmos_NotifyHandler = Cosmos_Notify_Callback;
    resp->Cosmos_ReleaseResultHandler = Cosmos_ReleaseResult_Callback;
    resp->Cosmos_InvokeHandler = Cosmos_Invoke_Callback;
    resp->Cosmos_ReleaseInvokeResponseHandler = Cosmos_ReleaseInvokeResponse_Callback;
    resp->Cosmos_SubscribeHandler = Cosmos_Subscribe_Callback;
    resp->Cosmos_UnsubscribeHandler = Cosmos_UnSubscribe_Callback;
    resp->Cosmos_PushSubscriptionDataHandler = Cosmos_PushSubscriptionData_Callback;
    resp->Cosmos_ReleaseSubscribeResponseHandler = Cosmos_ReleaseSubscribeResponse_Callback;

    g_envParams = new Cosmos_EnvironmentCreationParameters;
    memset(g_envParams, 0, sizeof(Cosmos_EnvironmentCreationParameters));
    g_envParams->Responsibility = resp;
    g_envParams->ClientParameters = clientParameters;
    g_envParams->DeveloperParameter = developerParameters;
    g_envParams->WebViewParameters = webViewParameters;

    Cosmos_Result* r = g_initEnv(nullptr, g_envParams, nullptr);
    if (!r || r->Code != 200) {
        int code = r ? static_cast<int>(r->Code) : -1;
        qDebug() << "Cosmos initialization failed, code =" << code;
    } else {
        qDebug() << "Cosmos initialization succeeded";
    }
    if (r) {
        // 初始化结果由 SDK 分配，不在此释放；保持与示例一致
    }
}

void MainWindow::destroyWidget()
{
    // 如果已有组件，先关闭它
    if (m_widgetHandle.empty() && m_windowHandle.empty()) {
        return;  // 没有已创建的组件，直接返回
    }

    if (!g_invoke) {
        qDebug() << "Cosmos SDK not initialized, cannot destroy widget";
        return;
    }

    // 构造 DestroyWidget 调用请求
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

    jsParameters.AddMember(rapidjson::StringRef("WidgetHandle"),
                          rapidjson::Value(m_widgetHandle.c_str(), doc.GetAllocator()),
                          doc.GetAllocator());
    jsParameters.AddMember(rapidjson::StringRef("WindowHandle"),
                          rapidjson::Value(m_windowHandle.c_str(), doc.GetAllocator()),
                          doc.GetAllocator());

    jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
    jsActionContext.AddMember(rapidjson::StringRef("Function"),
                             rapidjson::StringRef("DestroyWidget"), doc.GetAllocator());
    jsActionContext.AddMember(rapidjson::StringRef("Invoker"),
                             rapidjson::StringRef("00000000"), doc.GetAllocator());

    doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
    doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
    doc.AddMember(rapidjson::StringRef("ActionInstance"),
                 rapidjson::StringRef("QtHostDemo"), doc.GetAllocator());

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);

    std::string payloadUtf8 = gbk_to_utf8(strBuf.GetString());

    Cosmos_InvokeRequest* req = new Cosmos_InvokeRequest;
    std::string methodUtf8 = gbk_to_utf8("DestroyWidget");
    req->Method = methodUtf8.c_str();
    req->Parameters = payloadUtf8.c_str();

    Cosmos_InvokeResponse* resp = g_invoke(nullptr, req);
    if (resp) {
        int code = static_cast<int>(resp->Result->Code);
        if (code == 200) {
            qDebug() << "Widget destroyed successfully";
        } else {
            qDebug() << "DestroyWidget failed, code =" << code;
        }
        g_releaseInvoke(nullptr, resp);
    } else {
        qDebug() << "DestroyWidget call failed (null response)";
    }
    delete req;

    // 清理本地状态
    m_widgetHandle.clear();
    m_windowHandle.clear();

    // 清理 UI 对象（如果存在）
    if (m_embeddedWidget) {
        m_embeddedWidget->setParent(nullptr);
        delete m_embeddedWidget;
        m_embeddedWidget = nullptr;
        m_embeddedWindow = nullptr;  // 已被自动删除
    } else if (m_embeddedWindow) {
        delete m_embeddedWindow;
        m_embeddedWindow = nullptr;
    }
}

void MainWindow::shutdownCosmos()
{
    // 如果已经关闭过，直接返回
    if (m_cosmosShutdown) {
        return;
    }

    if (!g_invoke) {
        qDebug() << "Cosmos SDK not initialized, cannot shutdown";
        m_cosmosShutdown = true;
        return;
    }

    // 构造 ShutdownCosmos 调用请求
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value jsParameters(rapidjson::kObjectType), jsActionContext(rapidjson::kObjectType);

    jsActionContext.AddMember(rapidjson::StringRef("Parameters"), jsParameters, doc.GetAllocator());
    jsActionContext.AddMember(rapidjson::StringRef("Function"),
                             rapidjson::StringRef("ShutdownCosmos"), doc.GetAllocator());
    jsActionContext.AddMember(rapidjson::StringRef("Invoker"),
                             rapidjson::StringRef("00000000"), doc.GetAllocator());

    doc.AddMember(rapidjson::StringRef("ActionContext"), jsActionContext, doc.GetAllocator());
    doc.AddMember(rapidjson::StringRef("Action"), rapidjson::StringRef("Invoke"), doc.GetAllocator());
    doc.AddMember(rapidjson::StringRef("ActionInstance"),
                 rapidjson::StringRef("QtHostDemo"), doc.GetAllocator());

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);

    std::string payloadUtf8 = gbk_to_utf8(strBuf.GetString());

    Cosmos_InvokeRequest* req = new Cosmos_InvokeRequest;
    std::string methodUtf8 = gbk_to_utf8("ShutdownCosmos");
    req->Method = methodUtf8.c_str();
    req->Parameters = payloadUtf8.c_str();

    Cosmos_InvokeResponse* resp = g_invoke(nullptr, req);
    if (resp) {
        int code = static_cast<int>(resp->Result->Code);
        if (code == 200) {
            qDebug() << "Cosmos engine shutdown successfully";
        } else {
            qDebug() << "ShutdownCosmos failed, code =" << code;
        }
        g_releaseInvoke(nullptr, resp);
    } else {
        qDebug() << "ShutdownCosmos call failed (null response)";
    }
    delete req;

    m_cosmosShutdown = true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 在关闭窗口前，先关闭组件和组件引擎
    // 1. 如果有已创建的组件，先关闭它
    if (!m_widgetHandle.empty() || !m_windowHandle.empty()) {
        destroyWidget();
    }

    // 2. 关闭组件引擎
    shutdownCosmos();

    // 3. 允许窗口关闭
    event->accept();
}

void MainWindow::createAndEmbedWidget()
{
    if (!m_cosmosContainer || !g_invoke) {
        qDebug() << "Environment not initialized";
        return;
    }

    // 如果已有组件，先关闭它
    if (!m_widgetHandle.empty() || !m_windowHandle.empty()) {
        destroyWidget();
    }

    // 确保容器窗口已创建（调用 winId() 会强制创建窗口）
    WId parentWindowId = m_cosmosContainer->winId();
    if (!parentWindowId) {
        qDebug() << "Failed to get Qt container window ID";
        return;
    }

    // 构造 CreateWidget 调用请求
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value jsPreference(rapidjson::kObjectType),
                     jsParameters(rapidjson::kObjectType),
                     jsActionContext(rapidjson::kObjectType);

    // 将窗口 ID 转换为字符串
    // Windows: HWND (指针类型)
    // Linux: X11 Window (unsigned long)
    QString parentStr;
#ifdef Q_OS_WIN
    // Windows 上 WId 就是 HWND（指针），先转换为 void*，再转换为 quintptr，最后转换为 qint64
    void* hwndPtr = reinterpret_cast<void*>(parentWindowId);
    quintptr hwndValue = reinterpret_cast<quintptr>(hwndPtr);
    parentStr = QString::number(static_cast<qint64>(hwndValue));
#elif defined(Q_OS_LINUX)
    // Linux 上 WId 就是 X11 Window，先转换为 quintptr，再转换为 unsigned long
    quintptr widValue = reinterpret_cast<quintptr>(parentWindowId);
    parentStr = QString::number(static_cast<unsigned long>(widValue));
#else
    qDebug() << "Window embedding not supported on this platform";
    return;
#endif

    jsPreference.AddMember(rapidjson::StringRef("ParentHandle"),
                           rapidjson::Value(parentStr.toStdString().c_str(), doc.GetAllocator()),
                           doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("TitleBarVisibility"),
                           rapidjson::StringRef("Visible"), doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("WindowVisibility"),
                           rapidjson::StringRef("Visible"), doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("ResizeMode"),
                           rapidjson::StringRef("CanResize"), doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("WidgetWidth"),
                           600, doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("WidgetHeight"),
                           400, doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("BorderThickness"),
                           rapidjson::StringRef("1"), doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("WindowTop"),
                           0, doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("WindowLeft"),
                           0, doc.GetAllocator());

    jsParameters.AddMember(rapidjson::StringRef("WidgetPreference"),
                           jsPreference, doc.GetAllocator());
    jsParameters.AddMember(rapidjson::StringRef("WidgetGuid"),
                           rapidjson::StringRef("d8d4e7ca-5b8d-4396-bb14-7591fea00040"),
                           doc.GetAllocator());
    jsParameters.AddMember(rapidjson::StringRef("AppGuid"),
                           rapidjson::StringRef("2e05035e-9ce9-4f76-a5cb-9a8fff055361"),
                           doc.GetAllocator());

    jsActionContext.AddMember(rapidjson::StringRef("Parameters"),
                              jsParameters, doc.GetAllocator());
    jsActionContext.AddMember(rapidjson::StringRef("Invoker"),
                              rapidjson::StringRef("00000000"), doc.GetAllocator());
    jsActionContext.AddMember(rapidjson::StringRef("Function"),
                              rapidjson::StringRef("CreateWidget"), doc.GetAllocator());

    doc.AddMember(rapidjson::StringRef("ActionContext"),
                  jsActionContext, doc.GetAllocator());
    doc.AddMember(rapidjson::StringRef("Action"),
                  rapidjson::StringRef("Invoke"), doc.GetAllocator());
    doc.AddMember(rapidjson::StringRef("ActionInstance"),
                  rapidjson::StringRef("QtHostDemo"), doc.GetAllocator());

    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    doc.Accept(writer);

    std::string payloadUtf8 = gbk_to_utf8(strBuf.GetString());

    Cosmos_InvokeRequest* req = new Cosmos_InvokeRequest;
    std::string methodUtf8 = gbk_to_utf8("CreateWidget");
    req->Method = methodUtf8.c_str();
    req->Parameters = payloadUtf8.c_str();

    Cosmos_InvokeResponse* resp = g_invoke(nullptr, req);
    if (!resp || !resp->Result) {
        qDebug() << "CreateWidget call failed (null response)";
        delete req;
        return;
    }

    int code = static_cast<int>(resp->Result->Code);
    std::string dataStr;
    if (resp->DataFrame && resp->DataFrame->Data) {
        dataStr = resp->DataFrame->Data;
    }

    g_releaseInvoke(nullptr, resp);
    delete req;

    if (code != 200) {
        qDebug() << "CreateWidget failed, code =" << code;
        return;
    }

    // 解析响应 JSON（dataStr 已经是 UTF-8 编码的 JSON 字符串）
    rapidjson::Document docResult;
    docResult.Parse(dataStr.c_str());
    if (docResult.HasParseError()) {
        qDebug() << "CreateWidget result parse failed, error offset:" << docResult.GetErrorOffset();
        return;
    }

    // 提取 WidgetHandle 和 WindowHandle
    if (docResult.HasMember("ActionContext") && 
        docResult["ActionContext"].HasMember("Return")) {
        const auto& returnObj = docResult["ActionContext"]["Return"];
        
        if (returnObj.HasMember("WidgetHandle") && returnObj["WidgetHandle"].IsString()) {
            m_widgetHandle = returnObj["WidgetHandle"].GetString();
            qDebug() << "WidgetHandle:" << QString::fromStdString(m_widgetHandle);
        }
        
        if (returnObj.HasMember("WindowHandle") && returnObj["WindowHandle"].IsString()) {
            m_windowHandle = returnObj["WindowHandle"].GetString();
            qDebug() << "WindowHandle:" << QString::fromStdString(m_windowHandle);
            
            // 将窗口句柄字符串转换为数值
            bool ok = false;
            WId childWindowId = 0;
            
#ifdef Q_OS_WIN
            // Windows: 窗口句柄是 HWND (指针)，WId 在 Windows 上就是 HWND
            // 先将字符串转换为 quintptr（指针大小的整数），再转换为指针
            quintptr hwndValue = QString::fromStdString(m_windowHandle).toULongLong(&ok, 10);
            if (ok && hwndValue != 0) {
                // 在 Windows 上，WId 就是 HWND（void*），先转换为 void*，再转换为 WId
                void* hwndPtr = reinterpret_cast<void*>(hwndValue);
                childWindowId = reinterpret_cast<WId>(hwndPtr);
            }
#elif defined(Q_OS_LINUX)
            // Linux: X11 Window ID 是 unsigned long，先转换为 quintptr，再转换为 WId
            unsigned long x11WindowId = QString::fromStdString(m_windowHandle).toULong(&ok, 10);
            if (ok && x11WindowId != 0) {
                quintptr widValue = static_cast<quintptr>(x11WindowId);
                childWindowId = reinterpret_cast<WId>(widValue);
            }
#endif
            
            if (ok && childWindowId != 0) {
                // 使用 QWindow::fromWinId 创建 QWindow
                // 先清理旧的窗口（如果存在）
                if (m_embeddedWidget) {
                    // 删除 m_embeddedWidget 会自动删除它关联的 m_embeddedWindow
                    m_embeddedWidget->setParent(nullptr);
                    delete m_embeddedWidget;
                    m_embeddedWidget = nullptr;
                    m_embeddedWindow = nullptr;  // 已被自动删除，设置为 nullptr
                } else if (m_embeddedWindow) {
                    // 如果 m_embeddedWidget 不存在但 m_embeddedWindow 存在（createWindowContainer 失败的情况）
                    delete m_embeddedWindow;
                    m_embeddedWindow = nullptr;
                }
                
                m_embeddedWindow = QWindow::fromWinId(childWindowId);
                if (m_embeddedWindow) {
                    // 使用 createWindowContainer 将窗口嵌入到容器中
                    
                    m_embeddedWidget = QWidget::createWindowContainer(m_embeddedWindow, m_cosmosContainer);
                    if (m_embeddedWidget) {
                        // 设置布局，让嵌入的窗口填满容器
                        QLayout* existingLayout = m_cosmosContainer->layout();
                        if (!existingLayout) {
                            QVBoxLayout* containerLayout = new QVBoxLayout(m_cosmosContainer);
                            containerLayout->setContentsMargins(0, 0, 0, 0);
                            containerLayout->setSpacing(0);
                            m_cosmosContainer->setLayout(containerLayout);
                            containerLayout->addWidget(m_embeddedWidget);
                        } else {
                            // 如果已有布局，直接添加到布局中
                            existingLayout->addWidget(m_embeddedWidget);
                        }
                        
                        m_embeddedWidget->show();
                        m_embeddedWidget->setFocus();
                        
                        qDebug() << "Successfully embedded Cosmos window into Qt container, window ID:" << QString::fromStdString(m_windowHandle);
                    } else {
                        qDebug() << "createWindowContainer failed";
                        delete m_embeddedWindow;
                        m_embeddedWindow = nullptr;
                    }
                } else {
                    qDebug() << "QWindow::fromWinId failed, window ID:" << QString::fromStdString(m_windowHandle);
                }
            } else {
                qDebug() << "Window handle conversion failed:" << QString::fromStdString(m_windowHandle);
            }
        } else {
            qDebug() << "Response data missing WindowHandle field";
        }
    } else {
        qDebug() << "Response data format incorrect, missing ActionContext.Return";
    }

    qDebug() << "CreateWidget succeeded, component should be embedded in Qt container";
}

