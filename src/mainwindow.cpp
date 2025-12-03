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
#include <QComboBox>
#include <QFileDialog>

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // --- OpenGL Window ---
    m_glWindow = new OpenGLWindow();
    QWidget *glWidget = QWidget::createWindowContainer(m_glWindow, this);
    setCentralWidget(glWidget);

    // --- Menu ---
    QMenu *menuFile = menuBar()->addMenu("File");
    QAction *loadMesh3D = new QAction("Load Mesh 3D", this);
    menuFile->addAction(loadMesh3D);
    connect(loadMesh3D, &QAction::triggered, this, &mainWindow::openOffMesh);

    // --- Dock UI ---
    QDockWidget *dock = new QDockWidget("Scene Controls", this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);

    QWidget *dockContent = new QWidget();
    QFormLayout *layout = new QFormLayout();

    // --- ComboBox : Sélection du mesh ---
    meshSelector = new QComboBox();
    layout->addRow("Mesh sélectionné :", meshSelector);

    // Charger les meshes déjà présents dans la scène

    connect(m_glWindow, &OpenGLWindow::sceneReady, this, [this]() {

        meshSelector->clear();

        Scene* scene = m_glWindow->scene();
        if (!scene) return;

        const QVector<Mesh*>& meshes = scene->meshes();
        for (int i = 0; i < meshes.size(); ++i) {
            meshSelector->addItem(QString("Mesh %1").arg(i));
        }
    });


    connect(meshSelector, &QComboBox::currentIndexChanged,
            m_glWindow, &OpenGLWindow::setSelectedMesh);

    // --- Sliders Translation ---
    xSlider = new QSlider(Qt::Horizontal);
    ySlider = new QSlider(Qt::Horizontal);
    zSlider = new QSlider(Qt::Horizontal);

    xSlider->setRange(-10, 10);
    ySlider->setRange(-10, 10);
    zSlider->setRange(-10, 10);

    layout->addRow("Translation X :", xSlider);
    layout->addRow("Translation Y :", ySlider);
    layout->addRow("Translation Z :", zSlider);

    // --- Sliders Rotation ---
    rotationXslider = new QSlider(Qt::Horizontal);
    rotationYslider = new QSlider(Qt::Horizontal);
    rotationZslider = new QSlider(Qt::Horizontal);

    rotationXslider->setRange(0, 360);
    rotationYslider->setRange(0, 360);
    rotationZslider->setRange(0, 360);

    layout->addRow("Rotation X :", rotationXslider);
    layout->addRow("Rotation Y :", rotationYslider);
    layout->addRow("Rotation Z :", rotationZslider);

    // --- Scale ---
    scaleSlider = new QSlider(Qt::Horizontal);
    scaleSlider->setRange(1, 50);
    layout->addRow("Scale :", scaleSlider);

    // --- Reset Button ---
    QPushButton *resetBtn = new QPushButton("Reset Scene");
    layout->addRow(resetBtn);

    dockContent->setLayout(layout);
    dock->setWidget(dockContent);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // --- Connexions Sliders -> OpenGL ---
    connect(xSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setAxeX);
    connect(ySlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setAxeY);
    connect(zSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setAxeZ);

    connect(rotationXslider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRotationX);
    connect(rotationYslider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRotationY);
    connect(rotationZslider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRotationZ);

    connect(scaleSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setScale);

    connect(resetBtn, &QPushButton::clicked, this, &mainWindow::on_resetButton_clicked);
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

void mainWindow::on_resetButton_clicked()
{
    m_glWindow->resetScene();

    xSlider->setValue(0);
    ySlider->setValue(0);
    zSlider->setValue(0);

    rotationXslider->setValue(0);
    rotationYslider->setValue(0);
    rotationZslider->setValue(0);

    scaleSlider->setValue(10); // exemple : scale = 1.0f
}


