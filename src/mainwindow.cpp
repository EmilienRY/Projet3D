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
#include <QColorDialog>

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

    QDockWidget *dock = new QDockWidget("Scene Controls", this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    dock->setFixedWidth(350);

    QWidget *dockContent = new QWidget();
    QFormLayout *layout = new QFormLayout();

    meshSelector = new QComboBox();
    layout->addRow("Mesh sélectionné :", meshSelector);


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

    layout->addRow(QString("Translation X : %1").arg(xSlider->value()/10), xSlider);
    layout->addRow(QString("Translation Y : %1").arg(ySlider->value()/10), ySlider);
    layout->addRow(QString("Translation Z : %1").arg(zSlider->value()/10), zSlider);

    // --- Sliders Rotation ---
    rotationXslider = new QSlider(Qt::Horizontal);
    rotationYslider = new QSlider(Qt::Horizontal);
    rotationZslider = new QSlider(Qt::Horizontal);

    rotationXslider->setRange(0, 360);
    rotationYslider->setRange(0, 360);
    rotationZslider->setRange(0, 360);

    layout->addRow(QString("Rotation X : %1").arg(rotationXslider->value()), rotationXslider);
    layout->addRow(QString("Rotation Y : %1").arg(rotationYslider->value()), rotationYslider);
    layout->addRow(QString("Rotation Z : %1").arg(rotationZslider->value()), rotationZslider);

    // --- Scale ---
    scaleSlider = new QSlider(Qt::Horizontal);
    scaleSlider->setRange(1, 50);
    layout->addRow(QString("Scale : %1").arg(scaleSlider->value()/10), scaleSlider);

    connect(m_glWindow, &OpenGLWindow::selectedMeshChanged,
            this, &mainWindow::onMeshSelected);

    fpsLabel = new QLabel("FPS: 0");
    layout->addRow("FPS :", fpsLabel);
    connect(m_glWindow, &OpenGLWindow::fpsChanged, this, &mainWindow::updateFps);

    // --- Material ---

    QPushButton *btnColor = new QPushButton("Changer Couleur");
    layout->addRow("Couleur :", btnColor);

    connect(btnColor, &QPushButton::clicked, this, [this]() {

        QColorDialog *dialog = new QColorDialog(this);
        dialog->setOption(QColorDialog::DontUseNativeDialog, true);
        dialog->setWindowTitle("Choisir une couleur");

        connect(dialog, &QColorDialog::colorSelected,
                this, [this](const QColor &c) {
                    m_glWindow->changeColor(c);
                });

        dialog->open();
    });




    QPushButton *btnSpecularColor = new QPushButton("Changer Couleur reflets");
    layout->addRow("Couleur reflet :", btnSpecularColor);

    connect(btnSpecularColor, &QPushButton::clicked, this, [this]() {

        QColorDialog *dialog = new QColorDialog(this);
        dialog->setOption(QColorDialog::DontUseNativeDialog, true);
        dialog->setWindowTitle("Choisir une couleur");

        connect(dialog, &QColorDialog::colorSelected,
                this, [this](const QColor &c) {
                    m_glWindow->changeColorSpec(c);
                });

        dialog->open();
    });


    ksSlider = new QSlider(Qt::Horizontal);
    kdSlider = new QSlider(Qt::Horizontal);

    ksSlider->setRange(0, 100);
    kdSlider->setRange(0, 100);

    layout->addRow(QString("ks : %1").arg(ksSlider->value()/100), ksSlider);
    layout->addRow(QString("kd : %1").arg(kdSlider->value()/100), kdSlider);

    shininessSlider = new QSlider(Qt::Horizontal);
    shininessSlider->setRange(1, 200);
    layout->addRow(QString("Shininess : %1").arg(shininessSlider->value()), shininessSlider);

    // Light

    lightSelector = new QComboBox();
    layout->addRow("Light sélectionné :", lightSelector);


    connect(m_glWindow, &OpenGLWindow::sceneReady, this, [this]() {

        lightSelector->clear();

        Scene* scene = m_glWindow->scene();
        if (!scene) return;

        const QVector<Light>& lights = scene->lights();
        for (int i = 0; i < lights.size(); ++i) {
            lightSelector->addItem(QString("Light %1").arg(i));
        }
    });


    connect(lightSelector, &QComboBox::currentIndexChanged,
            m_glWindow, &OpenGLWindow::setSelectedLight);

    xLightSlider = new QSlider(Qt::Horizontal);
    yLightSlider = new QSlider(Qt::Horizontal);
    zLightSlider = new QSlider(Qt::Horizontal);

    xLightSlider->setRange(-50, 50);
    yLightSlider->setRange(-50, 50);
    zLightSlider->setRange(-50, 50);

    layout->addRow(QString("Light Translation X : %1").arg(xLightSlider->value()/10), xLightSlider);
    layout->addRow(QString("Light Translation Y : %1").arg(yLightSlider->value()/10), yLightSlider);
    layout->addRow(QString("Light Translation Z : %1").arg(zLightSlider->value()/10), zLightSlider);


    QPushButton *resetBtn = new QPushButton("Reset Scene");
    layout->addRow(resetBtn);

    dockContent->setLayout(layout);
    dock->setWidget(dockContent);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    connect(xSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setAxeX);
    connect(ySlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setAxeY);
    connect(zSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setAxeZ);

    connect(rotationXslider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRotationX);
    connect(rotationYslider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRotationY);
    connect(rotationZslider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRotationZ);

    connect(ksSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setKs);
    connect(kdSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setKd);

    connect(scaleSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setScale);
    connect(shininessSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setShininess);

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
        Material mat;
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        m_glWindow->loadOffFile(fileName, verts, idx, mat);

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
        Material mat;

        m_glWindow->scene()->loadObjFile(fileName, verts, idx, faceCount,mat);

        QMetaObject::invokeMethod(m_glWindow, [=]() {
            m_glWindow->openOBJmesh(fileName, verts, idx, faceCount);
            statusBar()->showMessage("Mesh loaded");
        });
    });
}

void mainWindow::on_resetButton_clicked()
{
    m_glWindow->resetScene();
    
    Scene* scene = m_glWindow->scene();
    if (!scene) return;

    const QVector<Mesh*>& meshes = scene->meshes();
    
    // Vérifier si la scène contient des meshes et si l'index est valide
    if (!meshes.isEmpty() && m_glWindow->m_selectedMesh >= 0 && m_glWindow->m_selectedMesh < meshes.size()) {
        xSlider->setValue(meshes[m_glWindow->m_selectedMesh]->position.x() * 10);
        ySlider->setValue(meshes[m_glWindow->m_selectedMesh]->position.y() * 10);
        zSlider->setValue(meshes[m_glWindow->m_selectedMesh]->position.z() * 10);

        rotationXslider->setValue(meshes[m_glWindow->m_selectedMesh]->rotation.x());
        rotationYslider->setValue(meshes[m_glWindow->m_selectedMesh]->rotation.y());
        rotationZslider->setValue(meshes[m_glWindow->m_selectedMesh]->rotation.z());

        scaleSlider->setValue(meshes[m_glWindow->m_selectedMesh]->scale * 10.0);

        kdSlider->setValue(meshes[m_glWindow->m_selectedMesh]->material().kd * 100);
        ksSlider->setValue(meshes[m_glWindow->m_selectedMesh]->material().ks * 100);

        shininessSlider->setValue(meshes[m_glWindow->m_selectedMesh]->material().shininess);
    }

    meshSelector->clear();
    
    for (int i = 0; i < meshes.size(); ++i) {
        if (meshes[i]->name != nullptr) {
            meshSelector->addItem(meshes[i]->name);
        }
        else{
            meshSelector->addItem(QString("Mesh %1").arg(i));
        }
    }
}

void mainWindow::onMeshSelected(int index, const QVector3D &pos, const QVector3D &rota, const float &scale, Material mat)
{
    QSignalBlocker b1(xSlider);
    QSignalBlocker b2(ySlider);
    QSignalBlocker b3(zSlider);

    xSlider->setValue(pos.x() * 10);
    ySlider->setValue(pos.y() * 10);
    zSlider->setValue(pos.z() * 10);

    rotationXslider->setValue(rota.x());
    rotationYslider->setValue(rota.y());
    rotationZslider->setValue(rota.z());

    scaleSlider->setValue(scale * 10.0);

    kdSlider->setValue(mat.kd *100);
    ksSlider->setValue(mat.ks *100);

    shininessSlider->setValue(mat.shininess);
}

void mainWindow::onLighthSelected(int index, const QVector3D &pos, const QVector3D &color, const float &intensity, float mat)
{
    QSignalBlocker b1(xLightSlider);
    QSignalBlocker b2(yLightSlider);
    QSignalBlocker b3(zLightSlider);

    xLightSlider->setValue(pos.x() * 10);
    yLightSlider->setValue(pos.y() * 10);
    zLightSlider->setValue(pos.z() * 10);

}

void mainWindow::updateFps(float fps)
{
    fpsLabel->setText(QString::number(fps, 'f', 2));
}


