#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}


