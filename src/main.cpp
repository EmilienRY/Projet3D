#include <QApplication>
#include "mainwindow.h"
#include <QSurfaceFormat>
#include <QProcessEnvironment>
#include <QByteArray>
#include <QProcess>
#include <QDebug>

int main(int argc, char *argv[])
{
    bool isPrimeSet = (qgetenv("__NV_PRIME_RENDER_OFFLOAD") == "1");
    bool isVendorSet = (qgetenv("__GLX_VENDOR_LIBRARY_NAME") == "nvidia");

    if (!isPrimeSet || !isVendorSet) {
        qDebug() << "NVIDIA offload not detected. Restarting with correct environment...";
        
        QString program = QString::fromLocal8Bit(argv[0]);
        QStringList args;
        for (int i = 1; i < argc; ++i) {
            args << QString::fromLocal8Bit(argv[i]);
        }

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("__NV_PRIME_RENDER_OFFLOAD", "1");
        env.insert("__GLX_VENDOR_LIBRARY_NAME", "nvidia");
        env.insert("__VK_LAYER_NV_optimus", "NVIDIA_only");
        env.insert("QT_XCB_GL_INTEGRATION", "xcb_glx");

        QProcess process;
        process.setProgram(program);
        process.setArguments(args);
        process.setProcessEnvironment(env);
        
        process.setProcessChannelMode(QProcess::ForwardedChannels);

        process.start();

        if (!process.waitForStarted()) {
            qCritical() << "Error: Failed to restart application:" << process.errorString();
            return 1;
        }

        process.waitForFinished(-1);
        
        return process.exitCode();
    }

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
