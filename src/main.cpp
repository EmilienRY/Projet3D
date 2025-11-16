#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    mainWindow window;
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
