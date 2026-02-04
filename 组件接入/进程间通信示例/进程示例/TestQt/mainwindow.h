#pragma once

#include <QMainWindow>
#include <QSet>
#include <QMap>
#include <QJsonObject>
#include <functional>

// 使用 PipeClient 提供的 C 接口作为底层 RPC 实现
#include "../PipeClient/PipeClient.h"

class QPushButton;
class QLineEdit;
class QComboBox;
class QTextEdit;
class QFile;
class QTextStream;

class RpcClient
{
public:
    bool initClient(const QString &pipeName,
                    const QString &logPath,
                    int logLevel)
    {
        Q_UNUSED(logLevel);
        const std::string pipe = pipeName.toStdString();
        const std::string log  = logPath.toStdString();
        bool ok = InitClient(pipe.c_str(),
                             static_cast<int>(pipe.size()),
                             log.c_str(),
                             static_cast<int>(log.size()),
                             2);
        m_inited = ok;
        return ok;
    }

    void notify(const QJsonObject &req)
    {
        if (!m_inited)
            return;
        const std::string id     = app::GetUuid();
        const std::string method = req.value(QStringLiteral("method")).toString().toStdString();

        Json::Value jsonParam = Json::objectValue;
        const auto paramVal = req.value(QStringLiteral("param"));
        if (paramVal.isObject()) {
            jsonFromQJson(paramVal.toObject(), jsonParam);
        }

        const std::string paramStr = jsonParam.toStyledString();
        void *rpcIn = CreateRpcRequest(id.c_str(), method.c_str(), paramStr.c_str());
        if (!rpcIn)
            return;
        Notify(rpcIn);
        FreeRpcAllocMemory(rpcIn);
    }

    void push(const QJsonObject &pushObj)
    {
        if (!m_inited)
            return;

        RpcPush push;
        push.topic = pushObj.value(QStringLiteral("topic")).toString().toStdString();

        const auto paramVal = pushObj.value(QStringLiteral("param"));
        if (!paramVal.isNull()) {
            Json::Value jsonParam;
            jsonFromQJson(paramVal.toObject(), jsonParam);
            push.param = jsonParam;
        }

        const std::string paramStr = push.param.toStyledString();
        void *rpcIn = CreateRpcPush(push.topic.c_str(), paramStr.c_str());
        if (!rpcIn)
            return;
        Push(rpcIn);
        FreeRpcAllocMemory(rpcIn);
    }

    int invoke(const QJsonObject &req, QJsonObject &resp, int timeoutMs = 30000)
    {
        if (!m_inited)
            return -1;

        const std::string id     = app::GetUuid();
        const std::string method = req.value(QStringLiteral("method")).toString().toStdString();

        const auto paramVal = req.value(QStringLiteral("param"));
        Json::Value jsonParam = Json::objectValue;
        if (!paramVal.isNull()) {
            jsonFromQJson(paramVal.toObject(), jsonParam);
        }

        const std::string paramStr = jsonParam.toStyledString();
        void *rpcIn = CreateRpcRequest(id.c_str(), method.c_str(), paramStr.c_str());
        if (!rpcIn)
            return -1;

        void *outPtr = nullptr;
        int outSize  = 0;
        RET_CALL ret = Invoke(rpcIn, &outPtr, &outSize, timeoutMs);
        FreeRpcAllocMemory(rpcIn);

        if (ret != RET_CALL::Ok || !outPtr || outSize <= 0) {
            if (outPtr)
                FreeRpcAllocMemory(outPtr);
            return -1;
        }

        std::string resultStr(static_cast<char*>(outPtr), outSize);
        FreeRpcAllocMemory(outPtr);

        // 解析 JSON 到 RpcResponse 结构再转为 QJsonObject
        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errs;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(resultStr.c_str(),
                           resultStr.c_str() + resultStr.size(),
                           &root, &errs)) {
            return -1;
        }

        RpcResponse out;
        out.id    = root.get("id", "").asString();
        out.code  = root.get("code", 0).asInt();
        out.error = root["error"];
        out.result = root["result"];

        resp["id"]   = QString::fromStdString(out.id);
        resp["code"] = out.code;

        QJsonObject resultObj;
        qjsonFromJson(out.result, resultObj);
        resp["result"] = resultObj;
        return out.code;
    }

    void invokeAsync(const QJsonObject &req,
                     std::function<void(int, QJsonObject)> cb)
    {
        // 简单使用同步 invoke 在线程中包装成异步
        std::thread([this, req, cb]() {
            QJsonObject resp;
            int ret = invoke(req, resp);
            cb(ret, resp);
        }).detach();
    }

    void notifyWidget(const QString &group,
                      const QString &type,
                      const QJsonObject &req)
    {
        if (!m_inited)
            return;

        const std::string method = req.value(QStringLiteral("method")).toString().toStdString();

        const auto paramVal = req.value(QStringLiteral("param"));
        Json::Value jsonParam = Json::objectValue;
        if (!paramVal.isNull()) {
            jsonFromQJson(paramVal.toObject(), jsonParam);
        }

        const std::string grp  = group.toStdString();
        const std::string typeStr = type.toStdString();
        InvokeType itype = InvokeType::Global;
        if (typeStr == "Group")
            itype = InvokeType::Group;
        else if (typeStr == "Instance")
            itype = InvokeType::Instance;

        const std::string paramStr = jsonParam.toStyledString();
        std::string id; // 通知可不需要 id
        void *rpcIn = CreateRpcRequest(id.c_str(), method.c_str(), paramStr.c_str());
        if (!rpcIn)
            return;

        NotifyWidget(grp.c_str(), itype, rpcIn, false);
        FreeRpcAllocMemory(rpcIn);
    }

    void invokeWidget(const QString &group,
                      const QString &type,
                      const QJsonObject &req,
                      std::function<void(int, QJsonObject)> cb)
    {
        if (!m_inited) {
            QJsonObject empty;
            cb(-1, empty);
            return;
        }

        const std::string id     = app::GetUuid();
        const std::string method = req.value(QStringLiteral("method")).toString().toStdString();

        const auto paramVal = req.value(QStringLiteral("param"));
        Json::Value jsonParam = Json::objectValue;
        if (!paramVal.isNull()) {
            jsonFromQJson(paramVal.toObject(), jsonParam);
        }

        const std::string grp  = group.toStdString();
        const std::string typeStr = type.toStdString();
        InvokeType itype = InvokeType::Global;
        if (typeStr == "Group")
            itype = InvokeType::Group;
        else if (typeStr == "Instance")
            itype = InvokeType::Instance;

        const std::string paramStr = jsonParam.toStyledString();
        void *rpcIn = CreateRpcRequest(id.c_str(), method.c_str(), paramStr.c_str());
        if (!rpcIn) {
            QJsonObject empty;
            cb(-1, empty);
            return;
        }

        std::thread([this, grp, itype, rpcIn, cb]() {
            void *outPtr = nullptr;
            int outSize  = 0;
            RET_CALL ret = InvokeWidget(grp.c_str(), itype, rpcIn, false, &outPtr, &outSize, 30000);

            QJsonObject respObj;
            int code = static_cast<int>(ret);

            if (ret == RET_CALL::Ok && outPtr && outSize > 0) {
                std::string resultStr(static_cast<char*>(outPtr), outSize);

                Json::CharReaderBuilder builder;
                Json::Value root;
                std::string errs;
                std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                if (reader->parse(resultStr.c_str(),
                                  resultStr.c_str() + resultStr.size(),
                                  &root, &errs)) {
                    QJsonObject resultQ;
                    qjsonFromJson(root, resultQ);
                    respObj = resultQ;
                }

                FreeRpcAllocMemory(outPtr);
            }

            cb(code, respObj);
            FreeRpcAllocMemory(rpcIn);
        }).detach();
    }

private:
    // QJsonObject <-> Json::Value 简单互转
    static void jsonFromQJson(const QJsonObject &src, Json::Value &dst)
    {
        dst = Json::Value(Json::objectValue);
        for (auto it = src.begin(); it != src.end(); ++it) {
            const QString &key = it.key();
            const QJsonValue &val = it.value();
            if (val.isString())
                dst[key.toStdString()] = val.toString().toStdString();
            else if (val.isDouble())
                dst[key.toStdString()] = val.toDouble();
            else if (val.isBool())
                dst[key.toStdString()] = val.toBool();
            else if (val.isObject()) {
                Json::Value child;
                jsonFromQJson(val.toObject(), child);
                dst[key.toStdString()] = child;
            }
            // 其他类型（数组等）如需用到可继续补充
        }
    }

    static void qjsonFromJson(const Json::Value &src, QJsonObject &dst)
    {
        dst = QJsonObject();
        const auto members = src.getMemberNames();
        for (const auto &name : members) {
            const Json::Value &v = src[name];
            QString key = QString::fromStdString(name);
            if (v.isString())
                dst.insert(key, QString::fromStdString(v.asString()));
            else if (v.isDouble() || v.isInt() || v.isUInt())
                dst.insert(key, v.asDouble());
            else if (v.isBool())
                dst.insert(key, v.asBool());
            else if (v.isObject()) {
                QJsonObject child;
                qjsonFromJson(v, child);
                dst.insert(key, child);
            }
        }
    }

    bool m_inited{false};
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void setPipeName(const QString &name) { m_pipeName = name; }
    void setWndInfo(const QString &info)  { m_wndInfo  = info; }

private slots:
    void onNotifyClicked();
    void onPushClicked();
    void onInvokeClicked();
    void onInvokeAsyncClicked();
    void onRequestThemeResClicked();
    void onRequestThemeClicked();
    void onGlobalSendClicked();
    void onGroupSendClicked();

private:
    void buildUi();
    void connectSignals();
    void connectPipe();
    void setupParentWindow();   // 使用宿主传入的窗口信息进行嵌入（仅在 Windows 下有效）
    void addStatusMessage(const QString &msg);
    void writeLog(const QString &msg);

    QJsonObject createRpcRequest(const QString &method,
                                 const QJsonObject &param = QJsonObject());

private:
    struct AccountInfo {
        QString id;
        int type   = 0;
        int status = 0;
    };

    // 与 Avalonia 示例对应的状态
    QString m_pipeName;
    QString m_wndInfo;
    bool    m_pipeSucc = false;

    static QSet<QString>              s_subSet;
    static QMap<QString, AccountInfo> s_accountMap;

    RpcClient m_rpc;

    // UI 控件
    QPushButton *m_btnNotify       = nullptr;
    QPushButton *m_btnPush         = nullptr;
    QPushButton *m_btnInvoke       = nullptr;
    QPushButton *m_btnInvokeAsync  = nullptr;
    QPushButton *m_btnReqThemeRes  = nullptr;
    QPushButton *m_btnReqTheme     = nullptr;
    QPushButton *m_btnDemoButton   = nullptr;
    QPushButton *m_btnGlobalSend   = nullptr;
    QPushButton *m_btnGroupSend    = nullptr;
    QLineEdit   *m_editContent     = nullptr;
    QLineEdit   *m_editGroup       = nullptr;
    QComboBox   *m_comboType       = nullptr;
    QTextEdit   *m_statusText      = nullptr;

    // 日志
    QFile       *m_logFile   = nullptr;
    QTextStream *m_logStream = nullptr;
};


