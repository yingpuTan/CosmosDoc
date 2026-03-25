#pragma once

#include <QMainWindow>

class QWidget;
class QWindow;
class QCloseEvent;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Cosmos notify 回调触发销毁逻辑时需要供外部 callback 使用
    void handleCosmosNotify(const char* topic, const char* message);

protected:
    void closeEvent(QCloseEvent *event) override;  // 重写关闭事件，确保在关闭前调用 ShutdownCosmos

private slots:
    void onCreateWidgetClicked();

private:
    void initCosmos();
    void createAndEmbedWidget();
    void destroyWidget();  // 关闭已创建的组件
    void shutdownCosmos();  // 关闭组件引擎

private:
    QWidget *m_cosmosContainer = nullptr;
    QPushButton *m_btnCreate = nullptr;
    QWidget *m_embeddedWidget = nullptr;  // 嵌入的外部窗口容器
    QWindow *m_embeddedWindow = nullptr;  // 嵌入的外部窗口
    std::string m_widgetHandle;           // 保存组件句柄，用于后续操作
    std::string m_windowHandle;           // 保存窗口句柄
    bool m_cosmosShutdown = false;        // 标记组件引擎是否已关闭
    bool m_embeddingInProgress = false;   // 防止重复触发创建过程

#ifdef Q_OS_LINUX
    // X11 下：CreateWidget 早期可能会先弹出一个顶层“壳窗口”，随后内容子窗才被嵌入 Qt。
    // 记录该壳窗口句柄，便于嵌入成功后隐藏，避免桌面残留弹窗。
    unsigned long m_x11ShellWindowId = 0;
#endif

};


