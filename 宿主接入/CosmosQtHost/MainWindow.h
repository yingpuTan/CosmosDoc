#pragma once

#include <QMainWindow>

class QWidget;
class QWindow;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

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
    QWidget *m_embeddedWidget = nullptr;  // 嵌入的外部窗口容器
    QWindow *m_embeddedWindow = nullptr;  // 嵌入的外部窗口
    std::string m_widgetHandle;           // 保存组件句柄，用于后续操作
    std::string m_windowHandle;           // 保存窗口句柄
    bool m_cosmosShutdown = false;        // 标记组件引擎是否已关闭
};


