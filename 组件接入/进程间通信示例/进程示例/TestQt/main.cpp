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

    // 仅兼容旧版：args[0] = pipeName, args[1] = wndInfo（与 TestWpf 一致）
    // Cosmos 进程侧按此顺序传参
    if (argc > 1) {
        pipeName = QString::fromLocal8Bit(argv[1]);
    }
    if (argc > 2) {
        wndInfo = QString::fromLocal8Bit(argv[2]);
    }

    MainWindow w;
    if (!pipeName.isEmpty())
        w.setPipeName(pipeName);
    if (!wndInfo.isEmpty())
        w.setWndInfo(wndInfo);

    w.show();
    return app.exec();
}


