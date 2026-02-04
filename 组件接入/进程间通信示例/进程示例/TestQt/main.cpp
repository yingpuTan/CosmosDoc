#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    QString pipeName;
    QString wndInfo;

    // 简单解析命令行参数：--pipe=xxx  --wnd-info=parent|w|h
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg.startsWith("--pipe=")) {
            pipeName = arg.mid(QStringLiteral("--pipe=").size());
        } else if (arg.startsWith("--wnd-info=")) {
            wndInfo = arg.mid(QStringLiteral("--wnd-info=").size());
        }
    }

    MainWindow w;
    if (!pipeName.isEmpty())
        w.setPipeName(pipeName);
    if (!wndInfo.isEmpty())
        w.setWndInfo(wndInfo);

    w.show();
    return app.exec();
}


