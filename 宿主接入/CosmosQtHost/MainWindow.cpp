
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QWindow>
#include <QDebug>

#include "Cosmos.Product.Sdk.h"
#include "platform.h"
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"

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
    return "Cosmos.MainApp";
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
    if (m_embeddedWidget) {
        m_embeddedWidget->setParent(nullptr);
        delete m_embeddedWidget;
        m_embeddedWidget = nullptr;
    }
    if (m_embeddedWindow) {
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
        qDebug() << "加载 Cosmos SDK 失败:" << QString::fromStdString(g_sdkModule.last_error());
        return;
    }

    // 2. 取出 C API 函数指针
    g_initEnv = reinterpret_cast<Cosmos_InitializeEnvironmentDelegate>(g_sdkModule.symbol("Cosmos_InitializeEnvironment"));
    g_uninitEnv = reinterpret_cast<Cosmos_UninitializeEnvironmentDelegate>(g_sdkModule.symbol("Cosmos_UninitializeEnvironment"));
    g_invoke = reinterpret_cast<Cosmos_InvokeDelegate>(g_sdkModule.symbol("Cosmos_Invoke"));
    g_releaseInvoke = reinterpret_cast<Cosmos_ReleaseInvokeResponseDelegate>(g_sdkModule.symbol("Cosmos_ReleaseInvokeResponse"));

    if (!g_initEnv || !g_uninitEnv || !g_invoke || !g_releaseInvoke) {
        qDebug() << "获取 Cosmos C API 函数指针失败";
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
        docProduct.AddMember(rapidjson::StringRef("Account"), "test", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Token"), "123", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Password"), "123123", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("ProductID"), "test", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("SpiderUrl"), "https://unitetest.chinastock.com.cn:8081", docProduct.GetAllocator());
        docProduct.AddMember(rapidjson::StringRef("Ip"), "10.4.124.34", docProduct.GetAllocator());
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

    static std::string idStr = std::string("HostDemo-```") + dataMarketAccount + "```" + dataProduct + "```";
    clientParameters->Id = idStr.c_str();
    clientParameters->Version = "0.0.0.1";
    clientParameters->HighDpiMode = Comos_GuiHighDpiMode::SystemAware;

    Cosmos_DeveloperParameter* developerParameters = new Cosmos_DeveloperParameter;
    memset(developerParameters, 0, sizeof(Cosmos_DeveloperParameter));
    developerParameters->AppProviderMode = "nuget;https://unitetest.chinastock.com.cn:453/v3/index.json";
    developerParameters->RuntimeMode = "debug";
    developerParameters->GuiMode = "show";

    Cosmos_WebViewParameters* webViewParameters = new Cosmos_WebViewParameters;
    memset(webViewParameters, 0, sizeof(Cosmos_WebViewParameters));
    webViewParameters->CefDirectory = "C:/Users/ThsQstudio";
    webViewParameters->CefResourcesDirectory = "C:/Users/ThsQstudio/Resources";
    webViewParameters->CefLocaleDirectory = "C:/Users/ThsQstudio/Resources/locales";

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
        qDebug() << "Cosmos 初始化失败，code =" << code;
    } else {
        qDebug() << "Cosmos 初始化成功";
    }
    if (r) {
        // 初始化结果由 SDK 分配，不在此释放；保持与示例一致
    }
}

void MainWindow::createAndEmbedWidget()
{
    if (!m_cosmosContainer || !g_invoke) {
        qDebug() << "环境未初始化完成";
        return;
    }

    // 确保容器窗口已创建（调用 winId() 会强制创建窗口）
    WId parentWindowId = m_cosmosContainer->winId();
    if (!parentWindowId) {
        qDebug() << "获取 Qt 容器窗口 ID 失败";
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
    // Windows 上 WId 就是 HWND，转换为 qint64 字符串
    parentStr = QString::number(reinterpret_cast<qint64>(parentWindowId));
#elif defined(Q_OS_LINUX)
    // Linux 上 WId 就是 X11 Window (unsigned long)，直接转换为字符串
    parentStr = QString::number(reinterpret_cast<unsigned long>(parentWindowId));
#else
    qDebug() << "当前平台不支持窗口嵌入";
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
                           rapidjson::StringRef("b0fd068e-2021-4619-acc0-53cda8d94a37"),
                           doc.GetAllocator());
    jsParameters.AddMember(rapidjson::StringRef("AppGuid"),
                           rapidjson::StringRef("1F74493F-E84D-4193-8FCE-F7CF4116EA63"),
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
        qDebug() << "CreateWidget 调用失败（返回空）";
        delete req;
        return;
    }

    int code = static_cast<int>(resp->Result->Code);
    std::string dataStr;
    if (resp->DataFrame && resp->DataFrame->Data && resp->DataFrame->DataSize > 0) {
        dataStr.assign(resp->DataFrame->Data, resp->DataFrame->DataSize);
    }

    g_releaseInvoke(nullptr, resp);
    delete req;

    if (code != 200) {
        qDebug() << "CreateWidget 失败, code =" << code;
        return;
    }

    // 解析响应 JSON（dataStr 已经是 UTF-8 编码的 JSON 字符串）
    rapidjson::Document docResult;
    docResult.Parse(dataStr.c_str());
    if (docResult.HasParseError()) {
        qDebug() << "CreateWidget 结果解析失败，错误位置:" << docResult.GetErrorOffset();
        return;
    }

    // 检查响应状态码
    if (!docResult.HasMember("Code") || docResult["Code"].GetInt64() != 200) {
        int64_t errorCode = docResult.HasMember("Code") ? docResult["Code"].GetInt64() : -1;
        qDebug() << "CreateWidget 返回错误码:" << errorCode;
        return;
    }

    if (!docResult.HasMember("Data") || !docResult["Data"].IsString()) {
        qDebug() << "CreateWidget 返回数据中没有 Data 字段或 Data 不是字符串";
        return;
    }

    // Data 字段是 JSON 字符串，需要再次解析
    std::string dataContent = docResult["Data"].GetString();
    rapidjson::Document docData;
    docData.Parse(dataContent.c_str());
    if (docData.HasParseError()) {
        qDebug() << "CreateWidget Data 字段解析失败，错误位置:" << docData.GetErrorOffset();
        qDebug() << "Data 内容:" << QString::fromStdString(dataContent);
        return;
    }

    // 提取 WidgetHandle 和 WindowHandle
    if (docData.HasMember("ActionContext") && 
        docData["ActionContext"].HasMember("Return")) {
        const auto& returnObj = docData["ActionContext"]["Return"];
        
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
            // Windows: 窗口句柄是 HWND (指针)，转换为 qint64
            qint64 hwndValue = QString::fromStdString(m_windowHandle).toLongLong(&ok, 10);
            if (ok && hwndValue != 0) {
                childWindowId = reinterpret_cast<WId>(reinterpret_cast<void*>(static_cast<quintptr>(hwndValue)));
            }
#elif defined(Q_OS_LINUX)
            // Linux: X11 Window ID 是 unsigned long
            unsigned long x11WindowId = QString::fromStdString(m_windowHandle).toULong(&ok, 10);
            if (ok && x11WindowId != 0) {
                childWindowId = reinterpret_cast<WId>(x11WindowId);
            }
#endif
            
            if (ok && childWindowId != 0) {
                // 使用 QWindow::fromWinId 创建 QWindow
                if (m_embeddedWindow) {
                    delete m_embeddedWindow;
                    m_embeddedWindow = nullptr;
                }
                
                m_embeddedWindow = QWindow::fromWinId(childWindowId);
                if (m_embeddedWindow) {
                    // 使用 createWindowContainer 将窗口嵌入到容器中
                    if (m_embeddedWidget) {
                        // 如果已经存在嵌入的窗口，先移除旧的
                        m_embeddedWidget->setParent(nullptr);
                        delete m_embeddedWidget;
                    }
                    
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
                        
                        qDebug() << "成功将 Cosmos 窗口嵌入到 Qt 容器中，窗口 ID:" << QString::fromStdString(m_windowHandle);
                    } else {
                        qDebug() << "createWindowContainer 失败";
                        delete m_embeddedWindow;
                        m_embeddedWindow = nullptr;
                    }
                } else {
                    qDebug() << "QWindow::fromWinId 失败，窗口 ID:" << QString::fromStdString(m_windowHandle);
                }
            } else {
                qDebug() << "窗口句柄转换失败:" << QString::fromStdString(m_windowHandle);
            }
        } else {
            qDebug() << "返回数据中没有 WindowHandle 字段";
        }
    } else {
        qDebug() << "返回数据格式不正确，缺少 ActionContext.Return";
    }

    qDebug() << "CreateWidget 成功，组件应已嵌入 Qt 容器";
}

