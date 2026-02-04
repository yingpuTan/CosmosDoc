#pragma once

#include <QMainWindow>

class QWidget;
class QWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onCreateWidgetClicked();

private:
    void initCosmos();
    void createAndEmbedWidget();

private:
    QWidget *m_cosmosContainer = nullptr;
    QWidget *m_embeddedWidget = nullptr;  // 嵌入的外部窗口容器
    QWindow *m_embeddedWindow = nullptr;  // 嵌入的外部窗口
    std::string m_widgetHandle;           // 保存组件句柄，用于后续操作
    std::string m_windowHandle;           // 保存窗口句柄
};


