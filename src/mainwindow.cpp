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
    QAction *loadOFF = new QAction("Load Mesh 3D (.off)", this);
    QAction *loadOBJ = new QAction("Load Mesh 3D (.obj)", this);
    menuFile->addAction(loadOFF);
    connect(loadOFF, &QAction::triggered, this, &mainWindow::openOffMesh);
    menuFile->addAction(loadOBJ);
    connect(loadOBJ, &QAction::triggered, this, &mainWindow::openObjMesh);

    // --- Dock UI ---
    QDockWidget *dock = new QDockWidget("Scene Controls", this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    dock->setFixedWidth(350);

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
            if (meshes[i]->name != nullptr) {
                meshSelector->addItem(meshes[i]->name);
            }
            else{
                meshSelector->addItem(QString("Mesh %1").arg(i));
            }
        }
    });


    connect(meshSelector, &QComboBox::currentIndexChanged,
            m_glWindow, &OpenGLWindow::setSelectedMesh);

    // --- Sliders Translation ---
    xSlider = new QSlider(Qt::Horizontal);
    ySlider = new QSlider(Qt::Horizontal);
    zSlider = new QSlider(Qt::Horizontal);

    xSlider->setRange(-50, 50);
    ySlider->setRange(-50, 50);
    zSlider->setRange(-50, 50);

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

    connect(m_glWindow, &OpenGLWindow::selectedMeshChanged,
            this, &mainWindow::onMeshSelected);


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
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setNameFilter("OFF Files (*.off)");
    dialog.setWindowTitle("Select 3D mesh");

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().first();

    QtConcurrent::run([this, fileName]() {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        m_glWindow->loadOffFile(fileName, verts, idx);

        QMetaObject::invokeMethod(this, [=]() {
            m_glWindow->openOffMesh(fileName, verts, idx);
            statusBar()->showMessage("Mesh loaded");
        });
    });
}

void mainWindow::openObjMesh()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setNameFilter("OBJ Files (*.obj)");
    dialog.setWindowTitle("Select 3D mesh");

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().first();


    QtConcurrent::run([this, fileName]() {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;
        int faceCount;

        m_glWindow->scene()->loadObjFile(fileName, verts, idx, faceCount);

        QMetaObject::invokeMethod(m_glWindow, [=]() {
            m_glWindow->openOBJmesh(fileName, verts, idx, faceCount);
            statusBar()->showMessage("Mesh loaded");
        });
    });
}

void mainWindow::on_resetButton_clicked()
{
    m_glWindow->resetScene();
    xSlider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->position.x()* 10);
    ySlider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->position.y()* 10);
    zSlider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->position.z()* 10);


    rotationXslider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->rotation.x());
    rotationYslider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->rotation.y());
    rotationZslider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->rotation.z());

    scaleSlider->setValue(m_glWindow->scene()->meshes()[m_glWindow->m_selectedMesh]->scale * 10.0);

    meshSelector->clear();

    Scene* scene = m_glWindow->scene();
    if (!scene) return;

    const QVector<Mesh*>& meshes = scene->meshes();
    for (int i = 0; i < meshes.size(); ++i) {
        if (meshes[i]->name != nullptr) {
            meshSelector->addItem(meshes[i]->name);
        }
        else{
            meshSelector->addItem(QString("Mesh %1").arg(i));
        }
    }
}

void mainWindow::onMeshSelected(int index, const QVector3D &pos, const QVector3D &rota, const float &scale)
{
    // Bloquer le signal des sliders pour éviter les boucles
    QSignalBlocker b1(xSlider);
    QSignalBlocker b2(ySlider);
    QSignalBlocker b3(zSlider);

    // Initialiser les sliders selon la position
    xSlider->setValue(pos.x() * 10);
    ySlider->setValue(pos.y() * 10);
    zSlider->setValue(pos.z() * 10);

    rotationXslider->setValue(rota.x());
    rotationYslider->setValue(rota.y());
    rotationZslider->setValue(rota.z());

    scaleSlider->setValue(scale * 10.0);
}


