#include "mainwindow.h"
#include "renderer/openglwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QtConcurrent>
#include <QWidget>
#include <QDockWidget>
#include <QFormLayout>
#include <QSlider>
#include <QPushButton>
#include <QLabel>

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_glWindow = new OpenGLWindow();

    QWidget *glWidget = QWidget::createWindowContainer(m_glWindow, this);
    setCentralWidget(glWidget);

    // --- Menu ---
    QMenu *menuFile = menuBar()->addMenu("File");
    QAction *loadMesh3D = new QAction("Load Mesh 3D", this);
    menuFile->addAction(loadMesh3D);
    connect(loadMesh3D, &QAction::triggered, this, &mainWindow::openOffMesh);

    // --- Interface à droite (Dock) ---
    QDockWidget *dock = new QDockWidget("Scene Controls", this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);

    QWidget *dockContent = new QWidget();
    QFormLayout *layout = new QFormLayout();

    // Slider pour la Translation
    QSlider *xSlider = new QSlider(Qt::Horizontal);
    QSlider *ySlider = new QSlider(Qt::Horizontal);
    QSlider *zSlider = new QSlider(Qt::Horizontal);

    xSlider->setRange(-10, 10);
    ySlider->setRange(-10, 10);
    zSlider->setRange(-10, 10);

    layout->addRow("Translation sur X:", xSlider);
    layout->addRow("Translation sur Y:", ySlider);
    layout->addRow("Translation sur Z:", zSlider);

    // Slider pour la Rotation
    QSlider *rotationXslider = new QSlider(Qt::Horizontal);
    QSlider *rotationYslider = new QSlider(Qt::Horizontal);
    QSlider *rotationZslider = new QSlider(Qt::Horizontal);

    rotationXslider->setRange(0, 360);
    rotationYslider->setRange(0, 360);
    rotationZslider->setRange(0, 360);

    layout->addRow("Rotation sur X:", rotationXslider);
    layout->addRow("Rotation sur X:", rotationYslider);
    layout->addRow("Rotation sur X:", rotationZslider);

    QSlider *scaleSlider = new QSlider(Qt::Horizontal);
    scaleSlider->setRange(0, 10);

    // Exemple : bouton reset
    QPushButton *resetBtn = new QPushButton("Reset Scene");
    layout->addRow(resetBtn);

    dockContent->setLayout(layout);
    dock->setWidget(dockContent);

    addDockWidget(Qt::RightDockWidgetArea, dock);

    // --- Connexions à OpenGL ---
    connect(xSlider, &QSlider::valueChanged,
            m_glWindow, &OpenGLWindow::setAxeX);

    connect(resetBtn, &QPushButton::clicked,
            m_glWindow, &OpenGLWindow::resetScene);
}

void mainWindow::openOffMesh()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, "Select 3D mesh", QString(), "OFF Files (*.off)");

    if (fileName.isEmpty())
        return;

    statusBar()->showMessage("Loading OFF...");

    QtConcurrent::run([this, fileName]() {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        m_glWindow->loadOffFile(fileName, verts, idx);

        QMetaObject::invokeMethod(this, [=]() {
            m_glWindow->openOffMesh(verts, idx);
            statusBar()->showMessage("Mesh loaded");
        });
    });
}


