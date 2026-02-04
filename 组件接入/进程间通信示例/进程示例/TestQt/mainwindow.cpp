#include "mainwindow.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QTextEdit>
#include <QScrollBar>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTime>
#include <QJsonDocument>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#ifdef Q_OS_LINUX
#  include <X11/Xlib.h>
#endif

QSet<QString> MainWindow::s_subSet;
QMap<QString, MainWindow::AccountInfo> MainWindow::s_accountMap;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 初始化日志
    m_logFile = new QFile(QStringLiteral("test_qt_status.log"), this);
    if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
        m_logStream = new QTextStream(m_logFile);
        *m_logStream << "========== 日志开始 "
                     << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
                     << " ==========\n";
        m_logStream->flush();
    }

    // 初始化全局账号信息
    AccountInfo info;
    info.id = QStringLiteral("123456");
    s_accountMap.insert(info.id, info);

    buildUi();
    connectSignals();

    setWindowTitle(QStringLiteral("Cosmos RPC Qt 测试 Demo"));
    resize(900, 500);

    // 如果上层通过命令行传入了父窗口信息，则尝试做窗口嵌入
    setupParentWindow();

    // 构造结束后尝试连接管道
    connectPipe();
}

MainWindow::~MainWindow()
{
    if (m_logStream) {
        *m_logStream << "========== 日志结束 ==========\n";
        m_logStream->flush();
    }
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *rootGrid = new QGridLayout(central);

    // 左侧布局
    auto *leftLayout = new QGridLayout;
    int row = 0;

    auto *lblAccount = new QLabel(QStringLiteral("测试账户"));
    QFont titleFont = lblAccount->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    lblAccount->setFont(titleFont);
    leftLayout->addWidget(lblAccount, row++, 0);

    m_btnNotify = new QPushButton(QStringLiteral("第一步：通知对方来查询和订阅"));
    m_btnNotify->setFixedHeight(43);
    leftLayout->addWidget(m_btnNotify, row++, 0);

    m_btnPush = new QPushButton(QStringLiteral("第二步：推送账户信息"));
    m_btnPush->setFixedHeight(44);
    leftLayout->addWidget(m_btnPush, row++, 0);

    auto *lblInvoke = new QLabel(QStringLiteral("Invoke测试（对方默认5秒返回）"));
    QFont subFont = lblInvoke->font();
    subFont.setPointSize(12);
    lblInvoke->setFont(subFont);
    leftLayout->addWidget(lblInvoke, row++, 0);

    m_btnInvoke = new QPushButton(QStringLiteral("同步Invoke示例"));
    m_btnInvoke->setFixedHeight(44);
    leftLayout->addWidget(m_btnInvoke, row++, 0);

    m_btnInvokeAsync = new QPushButton(QStringLiteral("异步Invoke示例"));
    m_btnInvokeAsync->setFixedHeight(44);
    leftLayout->addWidget(m_btnInvokeAsync, row++, 0);

    leftLayout->setRowStretch(row, 1);

    // 右侧布局
    auto *rightLayout = new QGridLayout;
    int rrow = 0;

    auto *lblUi = new QLabel(QStringLiteral("测试统一UI"));
    lblUi->setFont(titleFont);
    rightLayout->addWidget(lblUi, rrow++, 0, 1, 6);

    m_btnReqThemeRes = new QPushButton(QStringLiteral("第一步：获取皮肤资源"));
    m_btnReqThemeRes->setFixedHeight(41);
    rightLayout->addWidget(m_btnReqThemeRes, rrow++, 0, 1, 6);

    m_btnReqTheme = new QPushButton(QStringLiteral("第二步：获取当前皮肤主题"));
    m_btnReqTheme->setFixedHeight(40);
    rightLayout->addWidget(m_btnReqTheme, rrow++, 0, 1, 6);

    m_btnDemoButton = new QPushButton(QStringLiteral("效果展示按钮"));
    m_btnDemoButton->setFixedSize(115, 30);
    rightLayout->addWidget(m_btnDemoButton, rrow++, 0, 1, 6);

    auto *lblComm = new QLabel(QStringLiteral("组件通信透传测试"));
    lblComm->setFont(titleFont);
    rightLayout->addWidget(lblComm, rrow++, 0, 1, 6);

    // 行：发送内容 / 组名 / 类型
    auto *rowLayout = new QGridLayout;
    int c = 0;

    rowLayout->addWidget(new QLabel(QStringLiteral("发送内容:")), 0, c++);
    m_editContent = new QLineEdit;
    m_editContent->setFixedWidth(76);
    rowLayout->addWidget(m_editContent, 0, c++);

    rowLayout->addWidget(new QLabel(QStringLiteral("组名:")), 0, c++);
    m_editGroup = new QLineEdit;
    m_editGroup->setFixedWidth(76);
    rowLayout->addWidget(m_editGroup, 0, c++);

    m_comboType = new QComboBox;
    m_comboType->addItem(QStringLiteral("请求"));
    m_comboType->addItem(QStringLiteral("推送"));
    m_comboType->setFixedWidth(57);
    rowLayout->addWidget(m_comboType, 0, c++);

    rightLayout->addLayout(rowLayout, rrow++, 0, 1, 6);

    // 行：全局发送 / 组发送
    auto *btnRow = new QHBoxLayout;
    m_btnGlobalSend = new QPushButton(QStringLiteral("全局发送"));
    m_btnGroupSend  = new QPushButton(QStringLiteral("组发送"));
    btnRow->addWidget(m_btnGlobalSend);
    btnRow->addWidget(m_btnGroupSend);
    rightLayout->addLayout(btnRow, rrow++, 0, 1, 6);

    // 状态区域
    m_statusText = new QTextEdit;
    m_statusText->setReadOnly(true);
    m_statusText->setText(QStringLiteral("状态信息将显示在这里..."));
    rightLayout->addWidget(m_statusText, rrow++, 0, 1, 6);
    rightLayout->setRowStretch(rrow, 1);

    rootGrid->addLayout(leftLayout, 0, 0);
    rootGrid->addLayout(rightLayout, 0, 1);
    rootGrid->setColumnStretch(0, 1);
    rootGrid->setColumnStretch(1, 1);

    setCentralWidget(central);
}

void MainWindow::connectSignals()
{
    connect(m_btnNotify,      &QPushButton::clicked, this, &MainWindow::onNotifyClicked);
    connect(m_btnPush,        &QPushButton::clicked, this, &MainWindow::onPushClicked);
    connect(m_btnInvoke,      &QPushButton::clicked, this, &MainWindow::onInvokeClicked);
    connect(m_btnInvokeAsync, &QPushButton::clicked, this, &MainWindow::onInvokeAsyncClicked);
    connect(m_btnReqThemeRes, &QPushButton::clicked, this, &MainWindow::onRequestThemeResClicked);
    connect(m_btnReqTheme,    &QPushButton::clicked, this, &MainWindow::onRequestThemeClicked);
    connect(m_btnGlobalSend,  &QPushButton::clicked, this, &MainWindow::onGlobalSendClicked);
    connect(m_btnGroupSend,   &QPushButton::clicked, this, &MainWindow::onGroupSendClicked);
}

void MainWindow::connectPipe()
{
    if (m_pipeName.isEmpty()) {
        addStatusMessage(QStringLiteral("未提供管道名称，可在命令行通过 --pipe=xxx 指定"));
        return;
    }

    const QString logPath = QStringLiteral("test_rpc_qt.log");
    if (!m_rpc.initClient(m_pipeName, logPath, 2)) {
        addStatusMessage(QStringLiteral("连接失败！"));
        return;
    }

    m_pipeSucc = true;
    addStatusMessage(QStringLiteral("✓ 连接成功: ") + m_pipeName);

    // 与 Avalonia 示例保持一致：发送 init_succ
    QJsonObject req = createRpcRequest(QStringLiteral("init_succ"));
    m_rpc.notify(req);
}

void MainWindow::setupParentWindow()
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_LINUX)
    Q_UNUSED(m_wndInfo);
    return;
#else
    if (m_wndInfo.isEmpty())
        return;

    const QStringList parts = m_wndInfo.split(QLatin1Char('|'));
    if (parts.size() < 3)
        return;

    bool ok = false;
    qint64 parentHandleVal = parts[0].toLongLong(&ok);
    if (!ok || parentHandleVal == 0)
        return;

    int w = parts[1].toInt(&ok);
    if (!ok || w <= 0)
        w = width();
    int h = parts[2].toInt(&ok);
    if (!ok || h <= 0)
        h = height();

#ifdef Q_OS_WIN
    {
        HWND parentHwnd = reinterpret_cast<HWND>(static_cast<intptr_t>(parentHandleVal));
        if (!parentHwnd)
            return;

        // 强制创建本窗口的 WinId（即 HWND）
        HWND hwnd = reinterpret_cast<HWND>(winId());
        if (!hwnd)
            return;

        // 先去掉标题栏和任务栏显示，只作为嵌入子窗口出现
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

        // 设置父窗口与大小
        ::SetParent(hwnd, parentHwnd);
        ::MoveWindow(hwnd, 0, 0, w, h, TRUE);

        // 重新 show 一次以应用新的 flags
        show();
    }
#endif

#ifdef Q_OS_LINUX
    {
        // Qt 在 X11 下的窗口 ID 就是 X11 的 Window
        WId wid = winId();
        Window child = static_cast<Window>(wid);
        Window parent = static_cast<Window>(parentHandleVal);

        Display *dpy = XOpenDisplay(nullptr);
        if (!dpy)
            return;

        // 去掉边框等效果：Qt 侧使用无边框窗口
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

        // 把 Qt 窗口重挂到宿主父窗口上
        XReparentWindow(dpy, child, parent, 0, 0);
        XMoveResizeWindow(dpy, child, 0, 0, static_cast<unsigned int>(w), static_cast<unsigned int>(h));
        XMapRaised(dpy, child);
        XFlush(dpy);

        XCloseDisplay(dpy);

        // 重新 show 一次以应用新的 flags
        show();
    }
#endif
#endif // platform
}

QJsonObject MainWindow::createRpcRequest(const QString &method,
                                         const QJsonObject &param)
{
    QJsonObject obj;
    obj["method"] = method;
    obj["param"]  = param;
    return obj;
}

void MainWindow::addStatusMessage(const QString &msg)
{
    writeLog(msg);

    const QString line = QTime::currentTime().toString("HH:mm:ss")
                       + " - " + msg + "\n";
    m_statusText->moveCursor(QTextCursor::End);
    m_statusText->insertPlainText(line);
    m_statusText->verticalScrollBar()->setValue(
        m_statusText->verticalScrollBar()->maximum());
}

void MainWindow::writeLog(const QString &msg)
{
    if (!m_logStream)
        return;
    *m_logStream << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
                 << "] " << msg << "\n";
    m_logStream->flush();
}

// ================= 槽函数实现 =================

void MainWindow::onNotifyClicked()
{
    if (!m_pipeSucc) {
        addStatusMessage(QStringLiteral("请先建立连接"));
        return;
    }
    m_rpc.notify(createRpcRequest(QStringLiteral("notf_sub")));
    addStatusMessage(QStringLiteral("✓ Notify 发送成功"));
}

void MainWindow::onPushClicked()
{
    if (!m_pipeSucc) {
        addStatusMessage(QStringLiteral("请先建立连接"));
        return;
    }

    const QString subKey = QStringLiteral("sub_account_123456");
    if (!s_subSet.contains(subKey)) {
        addStatusMessage(QStringLiteral("请先订阅账户"));
        return;
    }

    QJsonObject push;
    push["topic"] = QStringLiteral("push_account");
    QJsonObject param;
    param["ID"]     = QStringLiteral("123456");
    param["Type"]   = 1;
    param["Status"] = 1;
    push["param"]   = param;

    m_rpc.push(push);
    addStatusMessage(QStringLiteral("✓ Push 发送成功"));
}

void MainWindow::onInvokeClicked()
{
    if (!m_pipeSucc) {
        addStatusMessage(QStringLiteral("请先建立连接"));
        return;
    }

    QJsonObject resp;
    int ret = m_rpc.invoke(createRpcRequest(QStringLiteral("test_invoke")), resp, 30000);
    const int code = resp.value(QStringLiteral("code")).toInt();
    if (ret == 0 && code == 0) {
        addStatusMessage(QStringLiteral("✓ 同步 Invoke 成功"));
        addStatusMessage(QStringLiteral("  响应 ID: ") + resp.value(QStringLiteral("id")).toString());
        addStatusMessage(QStringLiteral("  响应码: ") + QString::number(code));
    } else {
        addStatusMessage(QStringLiteral("✗ 同步 Invoke 失败，返回码: ") + QString::number(ret));
    }
}

void MainWindow::onInvokeAsyncClicked()
{
    if (!m_pipeSucc) {
        addStatusMessage(QStringLiteral("请先建立连接"));
        return;
    }

    m_rpc.invokeAsync(createRpcRequest(QStringLiteral("test_invoke_async")),
                      [this](int ret, const QJsonObject &resp) {
        const int code = resp.value(QStringLiteral("code")).toInt();
        if (ret == 0 && code == 0) {
            addStatusMessage(QStringLiteral("✓ 异步 Invoke 成功"));
            addStatusMessage(QStringLiteral("  响应 ID: ") + resp.value(QStringLiteral("id")).toString());
            addStatusMessage(QStringLiteral("  响应码: ") + QString::number(code));
        } else {
            addStatusMessage(QStringLiteral("✗ 异步 Invoke 失败，返回码: ") + QString::number(ret));
        }
    });

    addStatusMessage(QStringLiteral("异步测试已启动，不阻塞界面"));
}

void MainWindow::onRequestThemeResClicked()
{
    if (!m_pipeSucc) {
        addStatusMessage(QStringLiteral("请先建立连接"));
        return;
    }

    QJsonObject resp;
    int ret = m_rpc.invoke(createRpcRequest(QStringLiteral("requestThemeRes")), resp);
    const int code = resp.value(QStringLiteral("code")).toInt();
    if (ret == 0 && code == 0) {
        addStatusMessage(QStringLiteral("✓ 获取主题资源成功"));
    } else {
        addStatusMessage(QStringLiteral("✗ 获取主题资源失败"));
    }
}

void MainWindow::onRequestThemeClicked()
{
    if (!m_pipeSucc) {
        addStatusMessage(QStringLiteral("请先建立连接"));
        return;
    }

    QJsonObject resp;
    int ret = m_rpc.invoke(createRpcRequest(QStringLiteral("requestTheme")), resp);
    const int code = resp.value(QStringLiteral("code")).toInt();
    if (ret == 0 && code == 0) {
        const QJsonObject resultObj = resp.value(QStringLiteral("result")).toObject();
        const QString theme = resultObj.value(QStringLiteral("theme")).toString();
        addStatusMessage(QStringLiteral("✓ 获取主题成功: ") + theme);
        // TODO: 在这里根据 theme 应用 Qt 样式 / QSS
    } else {
        addStatusMessage(QStringLiteral("✗ 获取主题失败"));
    }
}

void MainWindow::onGlobalSendClicked()
{
    const QString group = m_editGroup->text();
    const bool isInvoke = (m_comboType->currentIndex() == 0);
    const QString text  = m_editContent->text();

    QJsonObject param;
    param["text"] = text;
    QJsonObject req = createRpcRequest(QStringLiteral("textchanged"), param);

    if (isInvoke) {
        m_rpc.invokeWidget(group, QStringLiteral("Global"), req,
                           [this](int ret, const QJsonObject &resp) {
            if (ret == 0) {
                addStatusMessage(QStringLiteral("全局请求成功: ")
                                 + resp.value(QStringLiteral("result")).toString());
            }
        });
    } else {
        m_rpc.notifyWidget(group, QStringLiteral("Global"), req);
        addStatusMessage(QStringLiteral("全局通知已发送"));
    }
}

void MainWindow::onGroupSendClicked()
{
    const QString group = m_editGroup->text();
    const bool isInvoke = (m_comboType->currentIndex() == 0);
    const QString text  = m_editContent->text();

    QJsonObject param;
    param["text"] = text;
    QJsonObject req = createRpcRequest(QStringLiteral("textchanged"), param);

    if (isInvoke) {
        m_rpc.invokeWidget(group, QStringLiteral("Group"), req,
                           [this](int ret, const QJsonObject &resp) {
            if (ret == 0) {
                addStatusMessage(QStringLiteral("组请求成功: ")
                                 + resp.value(QStringLiteral("result")).toString());
            }
        });
    } else {
        m_rpc.notifyWidget(group, QStringLiteral("Group"), req);
        addStatusMessage(QStringLiteral("组通知已发送"));
    }
}


