#include <QGuiApplication>
#include <QSurfaceFormat>
#include "renderer/openglwindow.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(4, 5);
    QSurfaceFormat::setDefaultFormat(fmt);

    OpenGLWindow window;
    window.setTitle("Qt + OpenGL 4.5 - Cube + Camera FPS");
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
