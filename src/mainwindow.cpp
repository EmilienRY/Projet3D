#include "mainwindow.h"
#include "renderer/openglwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QtConcurrent>
#include <QWidget>

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Crée ton OpenGLWindow
    m_glWindow = new OpenGLWindow();

    // Convertir QOpenGLWindow → widget
    QWidget *glWidget = QWidget::createWindowContainer(m_glWindow, this);
    setCentralWidget(glWidget);

    // ==== Menu ====
    QMenu *menuFile = menuBar()->addMenu("File");

    QAction *loadMesh3D = new QAction("Load Mesh 3D", this);
    menuFile->addAction(loadMesh3D);

    connect(loadMesh3D, &QAction::triggered, this, &mainWindow::openOffMesh);
}

void mainWindow::openOffMesh()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "Select 3D mesh", QString(), "OFF Files (*.off)");

    if (fileName.isEmpty())
        return;

    statusBar()->showMessage("Loading OFF...");

    // Lance le chargement en tâche de fond
    QtConcurrent::run([this, fileName]() {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        m_glWindow->loadOffFile(fileName, verts, idx);

        // Retourner dans le thread UI :
        QMetaObject::invokeMethod(this, [=]() {
            m_glWindow->openOffMesh(verts, idx);
            statusBar()->showMessage("Mesh loaded");
        });
    });
}


