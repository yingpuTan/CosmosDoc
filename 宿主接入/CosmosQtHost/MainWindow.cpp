
#include "MainWindow.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <QWindow>
#include <QCloseEvent>
#include <QDebug>
#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <algorithm>
#include <cstring>
#include <QMetaObject>

#include "Cosmos.Product.Sdk.h"
#include "platform.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#ifdef Q_OS_LINUX
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#endif

// ---------------- Cosmos C SDK 直接封装（最小化） ----------------

namespace {

platform::DynamicLibrary g_sdkModule;
Cosmos_EnvironmentCreationParameters* g_envParams = nullptr;
Cosmos_InitializeEnvironmentDelegate g_initEnv = nullptr;
Cosmos_UninitializeEnvironmentDelegate g_uninitEnv = nullptr;
Cosmos_InvokeDelegate g_invoke = nullptr;
Cosmos_ReleaseInvokeResponseDelegate g_releaseInvoke = nullptr;

MainWindow* g_mainWindow = nullptr;

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

#ifdef Q_OS_LINUX
bool QueryWindowParentX11(Window child, Window& parentOut)
{
    parentOut = 0;
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        qDebug() << "XOpenDisplay failed when querying parent window";
        return false;
    }

    Window root = 0;
    Window parent = 0;
    Window* children = nullptr;
    unsigned int nchildren = 0;
    const Status ok = XQueryTree(dpy, child, &root, &parent, &children, &nchildren);
    if (children) {
        XFree(children);
    }
    XCloseDisplay(dpy);

    if (!ok) {
        qDebug() << "XQueryTree failed for child window:" << static_cast<qulonglong>(child);
        return false;
    }

    parentOut = parent;
    return true;
}

bool EnsureWindowReparentedX11(WId childWindowId, WId parentWindowId, int width, int height)
{
    const Window child = static_cast<Window>(childWindowId);
    const Window parent = static_cast<Window>(parentWindowId);
    if (child == 0 || parent == 0) {
        qDebug() << "Invalid X11 window id, child:" << static_cast<qulonglong>(child)
                 << "parent:" << static_cast<qulonglong>(parent);
        return false;
    }

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        qDebug() << "XOpenDisplay failed when reparenting child window";
        return false;
    }

    constexpr int kMaxAttempts = 30;
    constexpr int kSleepMs = 100;
    bool success = false;

    for (int i = 0; i < kMaxAttempts; ++i) {
        Window currentParent = 0;
        if (QueryWindowParentX11(child, currentParent) && currentParent == parent) {
            success = true;
            qDebug() << "X11 parent already correct, child:" << static_cast<qulonglong>(child)
                     << "parent:" << static_cast<qulonglong>(parent)
                     << "attempt:" << (i + 1);
            break;
        }

        XReparentWindow(dpy, child, parent, 0, 0);
        XMoveResizeWindow(dpy, child, 0, 0,
                          static_cast<unsigned int>(std::max(1, width)),
                          static_cast<unsigned int>(std::max(1, height)));
        XMapRaised(dpy, child);
        XFlush(dpy);

        QCoreApplication::processEvents(QEventLoop::AllEvents, kSleepMs);
        QThread::msleep(kSleepMs);
    }

    XCloseDisplay(dpy);

    if (!success) {
        Window finalParent = 0;
        if (QueryWindowParentX11(child, finalParent)) {
            qDebug() << "X11 reparent timeout, child parent is"
                     << static_cast<qulonglong>(finalParent)
                     << "expected:" << static_cast<qulonglong>(parent);
        } else {
            qDebug() << "X11 reparent timeout and final parent query failed";
        }
    }
    return success;
}

bool X11WindowExists(Display* dpy, Window w)
{
    if (!dpy || !w) return false;
    XWindowAttributes attrs;
    // XGetWindowAttributes 在窗口不存在（BadWindow）时会失败，返回值为 0
    return XGetWindowAttributes(dpy, w, &attrs) != 0;
}

void LogX11WindowAttributes(Window w, const char* tag)
{
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    XWindowAttributes attrs;
    if (XGetWindowAttributes(dpy, w, &attrs)) {
        qDebug() << tag
                 << "winId:" << static_cast<qulonglong>(static_cast<quintptr>(w))
                 << "map_state:" << static_cast<int>(attrs.map_state)
                 << "geom:" << attrs.x << attrs.y << attrs.width << attrs.height;
    } else {
        qDebug() << tag << "XGetWindowAttributes failed for winId:"
                 << static_cast<qulonglong>(static_cast<quintptr>(w));
    }
    XCloseDisplay(dpy);
}

Window FindTopLevelX11(Display* dpy, Window w)
{
    if (!dpy || !w) return 0;

    Window current = w;
    for (int i = 0; i < 64; ++i) {
        Window root = 0;
        Window parent = 0;
        Window* children = nullptr;
        unsigned int nchildren = 0;
        const Status ok = XQueryTree(dpy, current, &root, &parent, &children, &nchildren);
        if (children) XFree(children);
        if (!ok) return current;
        if (parent == 0 || parent == root) return current;
        current = parent;
    }
    return current;
}

void UnmapWindowX11(Window w)
{
    if (!w) return;
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;
    // 对顶层窗口更友好：优先走 Withdraw（通知 WM），同时做多次 Unmap 兜底，
    // 避免被对端立即 remap 导致“弹窗不消失”。
    const int screen = DefaultScreen(dpy);
    XWithdrawWindow(dpy, w, screen);
    XFlush(dpy);

    for (int i = 0; i < 30; ++i) {
        XWindowAttributes attrs;
        const bool ok = XGetWindowAttributes(dpy, w, &attrs) != 0;
        if (!ok) break; // 窗口不存在了
        if (attrs.map_state != IsViewable) break; // 已经不可见
        XUnmapWindow(dpy, w);
        XFlush(dpy);
        QThread::msleep(50);
    }
    XCloseDisplay(dpy);
}

void RequestHideOrCloseX11Window(Window w, bool forceDestroy)
{
    if (!w) return;
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) return;

    Window root = DefaultRootWindow(dpy);

    // 1) 先尝试标准 EWMH：请求隐藏
    Atom netWmState = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom netWmStateHidden = XInternAtom(dpy, "_NET_WM_STATE_HIDDEN", False);
    if (netWmState != None && netWmStateHidden != None) {
        XClientMessageEvent ev;
        std::memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.window = root;
        ev.message_type = netWmState;
        ev.format = 32;
        ev.data.l[0] = 1; // 1: add
        ev.data.l[1] = netWmStateHidden;
        ev.data.l[2] = 0;
        ev.data.l[3] = 0;
        ev.data.l[4] = 0;
        XSendEvent(dpy, root, False, SubstructureRedirectMask | SubstructureNotifyMask,
                   reinterpret_cast<XEvent*>(&ev));
    }

    // 2) 再尝试标准 EWMH：请求关闭
    Atom netClose = XInternAtom(dpy, "_NET_CLOSE_WINDOW", False);
    if (netClose != None) {
        XClientMessageEvent ev;
        std::memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.window = root;
        ev.message_type = netClose;
        ev.format = 32;
        ev.data.l[0] = w;
        ev.data.l[1] = 0;
        ev.data.l[2] = 0;
        ev.data.l[3] = 0;
        ev.data.l[4] = 0;
        XSendEvent(dpy, root, False, SubstructureRedirectMask | SubstructureNotifyMask,
                   reinterpret_cast<XEvent*>(&ev));
    } else {
        // 3) 兼容：尝试 WM_DELETE_WINDOW
        Atom wmProtocols = XInternAtom(dpy, "WM_PROTOCOLS", False);
        Atom wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        if (wmProtocols != None && wmDelete != None) {
            int n = 0;
            Atom* protocols = nullptr;
            if (XGetWMProtocols(dpy, w, &protocols, &n) && protocols && n > 0) {
                bool supports = false;
                for (int i = 0; i < n; ++i) {
                    if (protocols[i] == wmDelete) {
                        supports = true;
                        break;
                    }
                }
                if (supports) {
                    XEvent ev;
                    std::memset(&ev, 0, sizeof(ev));
                    ev.xclient.type = ClientMessage;
                    ev.xclient.message_type = wmProtocols;
                    ev.xclient.display = dpy;
                    ev.xclient.window = w;
                    ev.xclient.format = 32;
                    ev.xclient.data.l[0] = wmDelete;
                    ev.xclient.data.l[1] = CurrentTime;
                    XSendEvent(dpy, w, False, NoEventMask, &ev);
                }
            }
            if (protocols) XFree(protocols);
        }
    }

    XFlush(dpy);

    // 4) 等一等：看能否变为不可见/被销毁
    for (int i = 0; i < 40; ++i) { // 4s 左右
        if (!X11WindowExists(dpy, w)) break;
        XWindowAttributes attrs;
        if (XGetWindowAttributes(dpy, w, &attrs)) {
            if (attrs.map_state != IsViewable) break;
        }
        QThread::msleep(100);
    }

    // 5) 兜底：如果还存在，按需强制销毁
    if (forceDestroy && X11WindowExists(dpy, w)) {
        XDestroyWindow(dpy, w);
        XFlush(dpy);
    }

    XCloseDisplay(dpy);
}
#endif

// 宿主回调：本 Demo 只简单返回成功，方便引擎初始化
Cosmos_Result* Cosmos_Notify_Callback(const Cosmos_CallContext* callContext, const Cosmos_NotifyRequest* notifyRequest)
{
    Q_UNUSED(callContext);
    Cosmos_Result* result = new Cosmos_Result;
    result->Code = 200;
    result->Message = "";

    if (g_mainWindow && notifyRequest) {
        const char* topic = notifyRequest->Topic ? notifyRequest->Topic : "";
        const char* message = notifyRequest->Message ? notifyRequest->Message : "";
        g_mainWindow->handleCosmosNotify(topic, message);
    }
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
    g_mainWindow = this;

    auto *central = new QWidget(this);
    auto *layout  = new QVBoxLayout(central);

    m_btnCreate = new QPushButton(QStringLiteral("创建并嵌入 Cosmos 组件"), this);
    m_cosmosContainer = new QWidget(this);
    m_cosmosContainer->setMinimumSize(600, 400);
    m_cosmosContainer->setStyleSheet(QStringLiteral("background-color:#202020;"));

    layout->addWidget(m_btnCreate);
    layout->addWidget(m_cosmosContainer, 1);
    setCentralWidget(central);

    connect(m_btnCreate, &QPushButton::clicked,
            this, &MainWindow::onCreateWidgetClicked);

    setWindowTitle(QStringLiteral("Cosmos Qt 宿主 Demo"));
    resize(1000, 700);

    initCosmos();
}

MainWindow::~MainWindow()
{
    if (g_mainWindow == this) {
        g_mainWindow = nullptr;
    }

    // 兜底：如果没有触发 closeEvent（或触发顺序异常），避免 Cosmos 侧窗口在 X11 上残留
    if (!m_cosmosShutdown) {
        if (g_invoke && (!m_widgetHandle.empty() || !m_windowHandle.empty())) {
            destroyWidget();
        }
        shutdownCosmos();
    }

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
#ifdef Q_OS_LINUX
        // 兜底：如果 Cosmos 句柄已清空但 X11 侧仍残留“壳窗”，也尝试清理
        if (m_x11ShellWindowId) {
            const Window shellWinId = static_cast<Window>(m_x11ShellWindowId);
            Display* dpy = XOpenDisplay(nullptr);
            if (dpy) {
                const bool exists = X11WindowExists(dpy, shellWinId);
                XCloseDisplay(dpy);
                if (exists) {
                    RequestHideOrCloseX11Window(shellWinId, true);
                }
            }
            m_x11ShellWindowId = 0;
        }
#endif
        return;
    }

    if (!g_invoke) {
        qDebug() << "Cosmos SDK not initialized, cannot destroy widget";
        return;
    }

    // 在清理本地状态前先解析 WindowHandle，用于 X11 销毁确认（避免“底窗残留”难以定位）
    WId childWindowIdForCheck = 0;
#ifdef Q_OS_LINUX
    {
        bool ok = false;
        unsigned long x11WindowId = QString::fromStdString(m_windowHandle).toULong(&ok, 10);
        if (ok && x11WindowId != 0) {
            childWindowIdForCheck = reinterpret_cast<WId>(static_cast<quintptr>(x11WindowId));
        }
    }
#endif

    // 先让 Qt 侧停止管理 native window，避免 Cosmos DestroyWidget 后 Qt 仍然去 Unmap/Reparent，
    // 从而触发 X11 的 BadWindow。
    if (m_embeddedWidget) {
        m_embeddedWidget->setParent(nullptr);
        delete m_embeddedWidget;
        m_embeddedWidget = nullptr;
        m_embeddedWindow = nullptr;
    } else if (m_embeddedWindow) {
        delete m_embeddedWindow;
        m_embeddedWindow = nullptr;
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

    // Linux/X11 下：偶现可能是 Cosmos 销毁存在异步延迟。
    // 做一次短轮询，确认 native window 是否还存在，便于你观测“底窗未销毁”的真实原因。
#ifdef Q_OS_LINUX
    if (childWindowIdForCheck) {
        Display* dpy = XOpenDisplay(nullptr);
        if (dpy) {
            const int kMaxAttempts = 30;  // 3s 左右
            bool alive = false;
            for (int i = 0; i < kMaxAttempts; ++i) {
                alive = X11WindowExists(dpy, static_cast<Window>(childWindowIdForCheck));
                if (!alive) break;
                QThread::msleep(100);
            }
            XCloseDisplay(dpy);
            if (alive) {
                qDebug() << "Warning: X11 window may still exist after DestroyWidget, winId:"
                         << static_cast<qulonglong>(reinterpret_cast<quintptr>(childWindowIdForCheck));
            }
        }
    }

    // 再兜底隐藏一次“壳窗口”，避免桌面残留弹窗。
    if (m_x11ShellWindowId) {
        const Window shellWinId = static_cast<Window>(m_x11ShellWindowId);
        m_x11ShellWindowId = 0; // 先清掉标记，避免后续逻辑重复处理

#ifdef Q_OS_LINUX
        // 先请求隐藏/关闭，不直接硬销毁，降低竞态下 BadWindow 概率
        if (XOpenDisplay(nullptr)) {
            // RequestHideOrClose 内部会做一定的存在性检查与必要时的兜底 destroy
            RequestHideOrCloseX11Window(shellWinId, false);
            // 再简单确认一次是否仍存在（有些 WM 会延迟处理）
            Display* dpy = XOpenDisplay(nullptr);
            if (dpy) {
                const int kMaxAttempts = 20;
                bool alive = true;
                for (int i = 0; i < kMaxAttempts; ++i) {
                    alive = X11WindowExists(dpy, shellWinId);
                    if (!alive) break;
                    QThread::msleep(100);
                }
                if (alive) {
                    RequestHideOrCloseX11Window(shellWinId, true);
                }
                XCloseDisplay(dpy);
            }
        }
#endif
    }
#endif

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

void MainWindow::handleCosmosNotify(const char* topic, const char* message)
{
    const std::string topicStr = topic ? topic : "";
    const std::string messageStr = message ? message : "";

    // 仅用于定位关闭/销毁触发点：你点击弹窗关闭按钮时，通常会触发 notify。
    qDebug() << "Cosmos Notify topic:" << QString::fromStdString(topicStr)
             << "message:" << QString::fromStdString(messageStr);

    if (m_cosmosShutdown) return;
    if (m_widgetHandle.empty() && m_windowHandle.empty()) return;

    // 尝试用关键词判断“关闭/销毁”类事件（中英文都兼容）。
    auto containsAny = [](const std::string& s, std::initializer_list<const char*> keys) -> bool {
        for (const char* k : keys) {
            if (!k) continue;
            if (s.find(k) != std::string::npos) return true;
        }
        return false;
    };

    const bool isCloseOrDestroy =
        containsAny(topicStr, {"Close", "close", "Closing", "Destroy", "destroy", "ShutDown", "Shutdown", "关闭", "销毁"}) ||
        containsAny(messageStr, {"Close", "close", "Closing", "Destroy", "destroy", "Shutdown", "关闭", "销毁"});

    if (!isCloseOrDestroy) return;

    // 回调通常不在 UI 线程执行；用 queued connection 确保 destroyWidget 在 Qt 线程执行。
    QMetaObject::invokeMethod(this, [this]() {
        if (!m_cosmosShutdown && (!m_widgetHandle.empty() || !m_windowHandle.empty())) {
            destroyWidget();
        }
    }, Qt::QueuedConnection);
}

void MainWindow::createAndEmbedWidget()
{
    if (m_embeddingInProgress) {
        qDebug() << "Embedding is already in progress, skip this request";
        return;
    }

    m_embeddingInProgress = true;
    if (m_btnCreate) {
        m_btnCreate->setEnabled(false);
    }
    auto finishEmbedding = [this]() {
        m_embeddingInProgress = false;
        if (m_btnCreate) {
            m_btnCreate->setEnabled(true);
        }
    };

    if (!m_cosmosContainer || !g_invoke) {
        qDebug() << "Environment not initialized";
        finishEmbedding();
        return;
    }

    // 如果已有组件，先关闭它
    if (!m_widgetHandle.empty() || !m_windowHandle.empty()) {
        destroyWidget();
    }
#ifdef Q_OS_LINUX
    // 兜底：若上一轮 Cosmos 句柄已清空但 X11 仍残留壳窗，也清理一下再创建
    if (m_x11ShellWindowId) {
        destroyWidget();
    }
#endif

    // 让容器先进入稳定状态，降低 Linux/X11 下 ParentHandle 刚创建即使用的竞态概率
    m_cosmosContainer->show();
    m_cosmosContainer->raise();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    // 确保容器窗口已创建（调用 winId() 会强制创建窗口）
    WId parentWindowId = m_cosmosContainer->winId();
    if (!parentWindowId) {
        qDebug() << "Failed to get Qt container window ID";
        finishEmbedding();
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
    finishEmbedding();
    return;
#endif

    jsPreference.AddMember(rapidjson::StringRef("ParentHandle"),
                           rapidjson::Value(parentStr.toStdString().c_str(), doc.GetAllocator()),
                           doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("TitleBarVisibility"),
                           rapidjson::StringRef("Hidden"), doc.GetAllocator());
    jsPreference.AddMember(rapidjson::StringRef("WindowVisibility"),
                           rapidjson::StringRef("Hidden"), doc.GetAllocator());
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
        finishEmbedding();
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
        finishEmbedding();
        return;
    }

    // 解析响应 JSON（dataStr 已经是 UTF-8 编码的 JSON 字符串）
    rapidjson::Document docResult;
    docResult.Parse(dataStr.c_str());
    if (docResult.HasParseError()) {
        qDebug() << "CreateWidget result parse failed, error offset:" << docResult.GetErrorOffset();
        finishEmbedding();
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
#ifdef Q_OS_LINUX
                // X11 下：CreateWidget 早期可能会先弹出一个顶层“壳窗口”，随后内容子窗才被嵌入 Qt。
                // 这里预先抓取该顶层窗口句柄，嵌入成功后主动隐藏，避免桌面残留弹窗。
                m_x11ShellWindowId = 0;
                {
                    Display* dpy = XOpenDisplay(nullptr);
                    if (dpy) {
                        const Window top = FindTopLevelX11(dpy, static_cast<Window>(childWindowId));
                        XCloseDisplay(dpy);
                        if (top && top != static_cast<Window>(childWindowId)) {
                            m_x11ShellWindowId = static_cast<unsigned long>(top);
                            qDebug() << "X11 shell(top-level) window id:" << static_cast<qulonglong>(top);
                        }
                    }
                }

                // 先验证/修正 X11 父子关系，再交给 Qt 包装，避免出现“窗口存在但不在容器里”
                const bool reparentOk = EnsureWindowReparentedX11(
                    childWindowId,
                    parentWindowId,
                    std::max(1, m_cosmosContainer->width()),
                    std::max(1, m_cosmosContainer->height()));
                if (!reparentOk) {
                    qDebug() << "Failed to reparent child window to Qt container on X11";
                    // 如果 Cosmos 已经创建了窗口，但我们无法完成嵌入，
                    destroyWidget();
                    finishEmbedding();
                    return;
                }
#endif

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
                        // 嵌入失败，避免 Cosmos 侧创建的原生窗口残留
                        destroyWidget();
                        finishEmbedding();
                        return;
                    }
                } else {
                    qDebug() << "QWindow::fromWinId failed, window ID:" << QString::fromStdString(m_windowHandle);
                    // 嵌入失败，避免 Cosmos 侧创建的原生窗口残留
                    destroyWidget();
                    finishEmbedding();
                    return;
                }
            } else {
                qDebug() << "Window handle conversion failed:" << QString::fromStdString(m_windowHandle);
                // 句柄转换失败也可能导致 Cosmos 创建了窗口但无法嵌入，做兜底销毁
                destroyWidget();
                finishEmbedding();
                return;
            }
        } else {
            qDebug() << "Response data missing WindowHandle field";
            // 既然 Cosmos 返回了组件创建成功信息，但缺少必要句柄，
            // 这里兜底触发销毁避免残留窗口。
            destroyWidget();
            finishEmbedding();
            return;
        }
    } else {
        qDebug() << "Response data format incorrect, missing ActionContext.Return";
        // 返回结构异常时可能仍然创建了组件窗口，尝试销毁兜底
        destroyWidget();
        finishEmbedding();
        return;
    }

    qDebug() << "CreateWidget succeeded, component should be embedded in Qt container";
    finishEmbedding();
}

