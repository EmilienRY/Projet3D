#include <QApplication>
#include "mainwindow.h"
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    mainWindow window;
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
