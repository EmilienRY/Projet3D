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
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QScrollArea>

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_glWindow = new OpenGLWindow();
    QWidget *glWidget = QWidget::createWindowContainer(m_glWindow, this);
    setCentralWidget(glWidget);

    QMenu *menuFile = menuBar()->addMenu("Fichier");
    QAction *loadOFF = new QAction("Charger Mesh (.off)", this);
    QAction *loadOBJ = new QAction("Charger Mesh (.obj)", this);
    menuFile->addAction(loadOFF);
    connect(loadOFF, &QAction::triggered, this, &mainWindow::openOffMesh);
    menuFile->addAction(loadOBJ);
    connect(loadOBJ, &QAction::triggered, this, &mainWindow::openObjMesh);

    QMenu *menuMesh = menuBar()->addMenu("Mesh");
    QAction *addSphere = new QAction("Ajouter une sphere", this);
    QAction *addPlane = new QAction("Ajouter un plan", this);
    menuMesh->addAction(addSphere);
    connect(addSphere, &QAction::triggered, this, &mainWindow::addSphereInScene);
    menuMesh->addAction(addPlane);
    connect(addPlane, &QAction::triggered, this, &mainWindow::addPlaneInScene);

    QMenu *menuLight = menuBar()->addMenu("Lumière");
    QAction *addLight = new QAction("Ajouter une lumière", this);
    menuLight->addAction(addLight);
    connect(addLight, &QAction::triggered, this, &mainWindow::addLight);

    QDockWidget *dock = new QDockWidget("Controle de la scène", this);
    dock->setAllowedAreas(Qt::RightDockWidgetArea);
    dock->setFixedWidth(450);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);

    QWidget *dockContent = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(dockContent);

    QGroupBox *meshGroup = new QGroupBox("Objets / Meshes");
    meshGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #555; border-radius: 5px; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; }");
    QFormLayout *meshLayout = new QFormLayout();

    meshSelector = new QComboBox();
    meshLayout->addRow("Sélection :", meshSelector);


    connect(m_glWindow, &OpenGLWindow::sceneReady, this, [this]() {
        {
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
    });


    connect(meshSelector, &QComboBox::currentIndexChanged,
            m_glWindow, &OpenGLWindow::setSelectedMesh);

    xSlider = new QSlider(Qt::Horizontal);
    ySlider = new QSlider(Qt::Horizontal);
    zSlider = new QSlider(Qt::Horizontal);

    xSlider->setRange(-100, 100);
    ySlider->setRange(-100, 100);
    zSlider->setRange(-100, 100);

    xLabel = new QLabel(QString::asprintf("Translation X : %5.1f", xSlider->value()/10.0));
    meshLayout->addRow(xLabel, xSlider);
    connect(xSlider, &QSlider::valueChanged, this, [this](int value){
        xLabel->setText(QString::asprintf("Translation X : %5.1f", value/10.0));
    });

    yLabel = new QLabel(QString::asprintf("Translation Y : %5.1f", ySlider->value()/10.0));
    meshLayout->addRow(yLabel, ySlider);
    connect(ySlider, &QSlider::valueChanged, this, [this](int value){
        yLabel->setText(QString::asprintf("Translation Y : %5.1f", value/10.0));
    });

    zLabel = new QLabel(QString::asprintf("Translation Z : %5.1f", zSlider->value()/10.0));
    meshLayout->addRow(zLabel, zSlider);
    connect(zSlider, &QSlider::valueChanged, this, [this](int value){
        zLabel->setText(QString::asprintf("Translation Z : %5.1f", value/10.0));
    });

    rotationXslider = new QSlider(Qt::Horizontal);
    rotationYslider = new QSlider(Qt::Horizontal);
    rotationZslider = new QSlider(Qt::Horizontal);

    rotationXslider->setRange(0, 360);
    rotationYslider->setRange(0, 360);
    rotationZslider->setRange(0, 360);

    rotXLabel = new QLabel(QString("Rotation X : %1").arg(rotationXslider->value()));
    meshLayout->addRow(rotXLabel, rotationXslider);
    connect(rotationXslider, &QSlider::valueChanged, this, [this](int value){
        rotXLabel->setText(QString("Rotation X : %1").arg(value));
    });

    rotYLabel = new QLabel(QString("Rotation Y : %1").arg(rotationYslider->value()));
    meshLayout->addRow(rotYLabel, rotationYslider);
    connect(rotationYslider, &QSlider::valueChanged, this, [this](int value){
        rotYLabel->setText(QString("Rotation Y : %1").arg(value));
    });

    rotZLabel = new QLabel(QString("Rotation Z : %1").arg(rotationZslider->value()));
    meshLayout->addRow(rotZLabel, rotationZslider);
    connect(rotationZslider, &QSlider::valueChanged, this, [this](int value){
        rotZLabel->setText(QString("Rotation Z : %1").arg(value));
    });

    scaleSlider = new QSlider(Qt::Horizontal);
    scaleSlider->setRange(1, 50);
    scaleLabel = new QLabel(QString("Scale : %1").arg(QString::number(scaleSlider->value()/10.0, 'f', 1)));
    meshLayout->addRow(scaleLabel, scaleSlider);
    connect(scaleSlider, &QSlider::valueChanged, this, [this](int value){
        scaleLabel->setText(QString("Scale : %1").arg(QString::number(value/10.0, 'f', 1)));
    });

    meshGroup->setLayout(meshLayout);
    mainLayout->addWidget(meshGroup);

    connect(m_glWindow, &OpenGLWindow::selectedMeshChanged,
            this, &mainWindow::onMeshSelected);

    connect(m_glWindow, &OpenGLWindow::selectedLightChanged,
            this, &mainWindow::onLighthSelected);

    QGroupBox *matGroup = new QGroupBox("Matériaux");
    matGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #555; border-radius: 5px; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; }");
    QFormLayout *matLayout = new QFormLayout();


    QPushButton *btnColor = new QPushButton("Changer la couleur");
    matLayout->addRow("Couleur :", btnColor);

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

    QPushButton *btnTexture = new QPushButton("Choisir texture");
    matLayout->addRow("Texture :", btnTexture);

    connect(btnTexture, &QPushButton::clicked, this, [this]() {
        QFileDialog dialog(this);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);
        dialog.setNameFilter("Images (*.png *.jpg *.jpeg)");
        dialog.setWindowTitle("Choisir une texture");

        if (dialog.exec() == QDialog::Accepted) {
            QString fileName = dialog.selectedFiles().first();
            m_glWindow->setTexture(fileName);
        }
    });

    QPushButton *btnRemoveTexture = new QPushButton("Supprimer texture");
    matLayout->addRow("", btnRemoveTexture);

    connect(btnRemoveTexture, &QPushButton::clicked, this, [this]() {
        m_glWindow->setTexture("");
    });

    QPushButton *btnNormalMap = new QPushButton("Choisir normal map");
    matLayout->addRow("Normal map :", btnNormalMap);

    connect(btnNormalMap, &QPushButton::clicked, this, [this]() {
        QFileDialog dialog(this);
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);
        dialog.setNameFilter("Images (*.png *.jpg *.jpeg)");
        dialog.setWindowTitle("Choisir une normal map");

        if (dialog.exec() == QDialog::Accepted) {
            QString fileName = dialog.selectedFiles().first();
            m_glWindow->setNormalMap(fileName);
        }
    });

    QPushButton *btnRemoveNormalMap = new QPushButton("Supprimer normal map");
    matLayout->addRow("", btnRemoveNormalMap);

    connect(btnRemoveNormalMap, &QPushButton::clicked, this, [this]() {
        m_glWindow->setNormalMap("");
    });


    QPushButton *btnSpecularColor = new QPushButton("Changer couleur reflets");
    matLayout->addRow("Couleur reflet :", btnSpecularColor);

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

    ksLabel = new QLabel(QString("Spéculaire (ks): %1").arg(QString::number(ksSlider->value()/100.0, 'f', 2)));
    matLayout->addRow(ksLabel, ksSlider);
    connect(ksSlider, &QSlider::valueChanged, this, [this](int value){
        ksLabel->setText(QString("Spéculaire (ks): %1").arg(QString::number(value/100.0, 'f', 2)));
    });

    kdLabel = new QLabel(QString("Diffuse (kd): %1").arg(QString::number(kdSlider->value()/100.0, 'f', 2)));
    matLayout->addRow(kdLabel, kdSlider);
    connect(kdSlider, &QSlider::valueChanged, this, [this](int value){
        kdLabel->setText(QString("Diffuse (kd): %1").arg(QString::number(value/100.0, 'f', 2)));
    });

    shininessSlider = new QSlider(Qt::Horizontal);
    shininessSlider->setRange(1, 200);
    shininessLabel = new QLabel(QString("Brillance : %1").arg(shininessSlider->value()));
    matLayout->addRow(shininessLabel, shininessSlider);
    connect(shininessSlider, &QSlider::valueChanged, this, [this](int value){
        shininessLabel->setText(QString("Brillance : %1").arg(value));
    });

    iorSlider = new QSlider(Qt::Horizontal);
    iorSlider->setRange(100, 300);
    iorSlider->setValue(150);
    iorLabel = new QLabel(QString("Indice réfraction (verre) : %1").arg(QString::number(iorSlider->value()/100.0, 'f', 2)));
    matLayout->addRow(iorLabel, iorSlider);
    connect(iorSlider, &QSlider::valueChanged, this, [this](int value){
        iorLabel->setText(QString("Indice réfraction (verre) : %1").arg(QString::number(value/100.0, 'f', 2)));
    });
    connect(iorSlider, &QSlider::valueChanged, this, [this](int value){
        m_glWindow->setIor(value/100.0f);
    });

    QPushButton *btnEmissionColor = new QPushButton("Changer couleur émission");
    matLayout->addRow("Couleur émission :", btnEmissionColor);

    connect(btnEmissionColor, &QPushButton::clicked, this, [this]() {
        QColorDialog *dialog = new QColorDialog(this);
        dialog->setOption(QColorDialog::DontUseNativeDialog, true);
        dialog->setWindowTitle("Choisir une couleur d'émission");

        connect(dialog, &QColorDialog::colorSelected,
                this, [this](const QColor &c) {
                    m_glWindow->setEmissionColor(c);
                });

        dialog->open();
    });

    emissionStrengthSlider = new QSlider(Qt::Horizontal);
    emissionStrengthSlider->setRange(0, 1000);
    emissionStrengthLabel = new QLabel(QString("Intensité émission : %1").arg(QString::number(emissionStrengthSlider->value()/10.0, 'f', 1)));
    matLayout->addRow(emissionStrengthLabel, emissionStrengthSlider);
    connect(emissionStrengthSlider, &QSlider::valueChanged, this, [this](int value){
        emissionStrengthLabel->setText(QString("Intensité émission : %1").arg(QString::number(value/10.0, 'f', 1)));
        m_glWindow->setEmissionStrength(value);
    });

    typeMat = new QComboBox(this);
    matLayout->addRow("Type de matériel :", typeMat);

    typeMat->addItem("Diffus");
    typeMat->addItem("Mirroir");
    typeMat->addItem("Verre");

    connect(typeMat, &QComboBox::currentIndexChanged,
            m_glWindow, &OpenGLWindow::setSelectedTypeMat);

    matGroup->setLayout(matLayout);
    mainLayout->addWidget(matGroup);

    QGroupBox *lightGroup = new QGroupBox("Lumières");
    lightGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #555; border-radius: 5px; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; }");
    QFormLayout *lightLayout = new QFormLayout();

    lightSelector = new QComboBox();
    lightLayout->addRow("Lumière sélectionnée :", lightSelector);


    connect(m_glWindow, &OpenGLWindow::sceneReady, this, [this]() {
        {
            QSignalBlocker blocker(lightSelector);
            lightSelector->clear();

            Scene* scene = m_glWindow->scene();
            if (!scene) return;

            const QVector<Light>& lights = scene->lights();
            for (int i = 0; i < lights.size(); ++i) {
                lightSelector->addItem(QString("Light %1").arg(i));
            }
        }
        if (lightSelector->count() > 0) {
            lightSelector->setCurrentIndex(0);
            m_glWindow->setSelectedLight(0);
        }
    });


    connect(lightSelector, &QComboBox::currentIndexChanged,
            m_glWindow, &OpenGLWindow::setSelectedLight);

    xLightSlider = new QSlider(Qt::Horizontal);
    yLightSlider = new QSlider(Qt::Horizontal);
    zLightSlider = new QSlider(Qt::Horizontal);

    xLightSlider->setRange(-100, 100);
    yLightSlider->setRange(-100, 100);
    zLightSlider->setRange(-100, 100);

    lightXLabel = new QLabel(QString("Lumière translation X : %1").arg(QString::number(xLightSlider->value()/10.0, 'f', 1)));
    lightLayout->addRow(lightXLabel, xLightSlider);
    connect(xLightSlider, &QSlider::valueChanged, this, [this](int value){
        lightXLabel->setText(QString("Lumière translation X : %1").arg(QString::number(value/10.0, 'f', 1)));
    });

    lightYLabel = new QLabel(QString("Lumière translation Y : %1").arg(QString::number(yLightSlider->value()/10.0, 'f', 1)));
    lightLayout->addRow(lightYLabel, yLightSlider);
    connect(yLightSlider, &QSlider::valueChanged, this, [this](int value){
        lightYLabel->setText(QString("Lumière translation Y : %1").arg(QString::number(value/10.0, 'f', 1)));
    });

    lightZLabel = new QLabel(QString("Lumière translation Z : %1").arg(QString::number(zLightSlider->value()/10.0, 'f', 1)));
    lightLayout->addRow(lightZLabel, zLightSlider);
    connect(zLightSlider, &QSlider::valueChanged, this, [this](int value){
        lightZLabel->setText(QString("Lumière translation Z : %1").arg(QString::number(value/10.0, 'f', 1)));
    });

    QPushButton *btnLightColor = new QPushButton("Changer couleur lumière");
    lightLayout->addRow("Couleur de la lumière :", btnLightColor);

    connect(btnLightColor, &QPushButton::clicked, this, [this]() {

        QColorDialog *dialog = new QColorDialog(this);
        dialog->setOption(QColorDialog::DontUseNativeDialog, true);
        dialog->setWindowTitle("Choisir une couleur");

        connect(dialog, &QColorDialog::colorSelected,
                this, [this](const QColor &c) {
                    m_glWindow->changeColorLight(c);
                });

        dialog->open();
    });


    lightIntensitySlider = new QSlider(Qt::Horizontal);
    lighRadiusSlider = new QSlider(Qt::Horizontal);

    lightIntensitySlider->setRange(1, 1000);
    lighRadiusSlider->setRange(0, 100);

    intensityLabel = new QLabel(QString("Intensitée : %1").arg(QString::number(lightIntensitySlider->value()/10.0, 'f', 1)));
    lightLayout->addRow(intensityLabel, lightIntensitySlider);
    connect(lightIntensitySlider, &QSlider::valueChanged, this, [this](int value){
        intensityLabel->setText(QString("Intensitée : %1").arg(QString::number(value/10.0, 'f', 1)));
    });

    radiusLabel = new QLabel(QString("Rayon lumière : %1").arg(QString::number(lighRadiusSlider->value()/10.0, 'f', 1)));
    lightLayout->addRow(radiusLabel, lighRadiusSlider);
    connect(lighRadiusSlider, &QSlider::valueChanged, this, [this](int value){
        radiusLabel->setText(QString("Rayon lumière : %1").arg(QString::number(value/10.0, 'f', 1)));
    });

    lightGroup->setLayout(lightLayout);
    mainLayout->addWidget(lightGroup);

    QGroupBox *globalGroup = new QGroupBox("Global");
    globalGroup->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #555; border-radius: 5px; margin-top: 10px; } QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; padding: 0 5px; }");
    QFormLayout *globalLayout = new QFormLayout();

    fpsLabel = new QLabel("FPS: 0");
    globalLayout->addRow("FPS :", fpsLabel);
    connect(m_glWindow, &OpenGLWindow::fpsChanged, this, &mainWindow::updateFps);

    // Max rebond
    QSlider *maxBouncesSlider = new QSlider(Qt::Horizontal);
    maxBouncesSlider->setRange(0, 20);
    maxBouncesSlider->setValue(4);
    QLabel *maxBouncesLabel = new QLabel(QString("Rebonds max : %1").arg(maxBouncesSlider->value()));
    globalLayout->addRow(maxBouncesLabel, maxBouncesSlider);
    connect(maxBouncesSlider, &QSlider::valueChanged, this, [maxBouncesLabel](int value){
        maxBouncesLabel->setText(QString("Rebonds max : %1").arg(value));
    });
    connect(maxBouncesSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setMaxBounces);

    // nb ray ombre
    QSlider *shadowSamplesSlider = new QSlider(Qt::Horizontal);
    shadowSamplesSlider->setRange(1, 32);
    shadowSamplesSlider->setValue(1);
    QLabel *shadowSamplesLabel = new QLabel(QString("Rayons d'ombres : %1").arg(shadowSamplesSlider->value()));
    globalLayout->addRow(shadowSamplesLabel, shadowSamplesSlider);
    connect(shadowSamplesSlider, &QSlider::valueChanged, this, [shadowSamplesLabel](int value){
        shadowSamplesLabel->setText(QString("Rayons d'ombres : %1").arg(value));
    });
    connect(shadowSamplesSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setShadowSamples);

    // nb rayons par pixels
    QSlider *sppSlider = new QSlider(Qt::Horizontal);
    sppSlider->setRange(1, 16);
    sppSlider->setValue(1);
    QLabel *sppLabel = new QLabel(QString("Rayons par pixels : %1").arg(sppSlider->value()));
    globalLayout->addRow(sppLabel, sppSlider);
    connect(sppSlider, &QSlider::valueChanged, this, [sppLabel](int value){
        sppLabel->setText(QString("Rayons par pixels : %1").arg(value));
    });
    connect(sppSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setSpp);

    // Denoise Mode
    QComboBox *denoiseModeCombo = new QComboBox();
    denoiseModeCombo->addItem("Accumulation + Débruitage");
    denoiseModeCombo->addItem("Accumulation");
    globalLayout->addRow("Mode de débruitage :", denoiseModeCombo);
    connect(denoiseModeCombo, &QComboBox::currentIndexChanged, m_glWindow, &OpenGLWindow::setDenoiseMode);

    // Denoise Strength
    QSlider *denoiseStrengthSlider = new QSlider(Qt::Horizontal);
    denoiseStrengthSlider->setRange(1, 20);
    denoiseStrengthSlider->setValue(5); 
    QLabel *denoiseStrengthLabel = new QLabel(QString("Intensité du débruitage : %1").arg(denoiseStrengthSlider->value()/10.0));
    globalLayout->addRow(denoiseStrengthLabel, denoiseStrengthSlider);
    connect(denoiseStrengthSlider, &QSlider::valueChanged, this, [denoiseStrengthLabel](int value){
        denoiseStrengthLabel->setText(QString("Intensité du débruitage : %1").arg(value/10.0));
    });
    connect(denoiseStrengthSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setDenoiseStrength);

    // Denoise Passes
    QSlider *denoisePassesSlider = new QSlider(Qt::Horizontal);
    denoisePassesSlider->setRange(1, 5);
    denoisePassesSlider->setValue(3);
    QLabel *denoisePassesLabel = new QLabel(QString("Passes de débruitage : %1").arg(denoisePassesSlider->value()));
    globalLayout->addRow(denoisePassesLabel, denoisePassesSlider);
    connect(denoisePassesSlider, &QSlider::valueChanged, this, [denoisePassesLabel](int value){
        denoisePassesLabel->setText(QString("Passes de débruitage : %1").arg(value));
    });
    connect(denoisePassesSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setDenoisePasses);

    // Background Color
    QPushButton *btnBgColor = new QPushButton("Changer couleur fond");
    globalLayout->addRow("Couleur de fond :", btnBgColor);

    connect(btnBgColor, &QPushButton::clicked, this, [this]() {
        QColorDialog *dialog = new QColorDialog(this);
        dialog->setOption(QColorDialog::DontUseNativeDialog, true);
        dialog->setWindowTitle("Choisir une couleur de fond");
        dialog->setCurrentColor(QColor(51, 76, 178)); 

        connect(dialog, &QColorDialog::colorSelected,
                this, [this](const QColor &c) {
                    m_glWindow->setBackgroundColor(c);
                });

        dialog->open();
    });

    QSlider *exposureSlider = new QSlider(Qt::Horizontal);
    exposureSlider->setRange(1, 100); 
    exposureSlider->setValue(10); 
    QLabel *exposureLabel = new QLabel(QString("Exposition : %1").arg(QString::number(exposureSlider->value()/10.0, 'f', 1)));
    globalLayout->addRow(exposureLabel, exposureSlider);
    connect(exposureSlider, &QSlider::valueChanged, this, [this, exposureLabel](int value){
        exposureLabel->setText(QString("Exposition : %1").arg(QString::number(value/10.0, 'f', 1)));
        m_glWindow->setExposure(value/10.0f);
    });

    QCheckBox *dofCheckbox = new QCheckBox("Activer profondeur de champs");
    dofCheckbox->setChecked(false);
    globalLayout->addRow(dofCheckbox);

    QSlider *lensRadiusSlider = new QSlider(Qt::Horizontal);
    lensRadiusSlider->setRange(0, 100);
    lensRadiusSlider->setValue(0);
    QLabel *lensRadiusLabel = new QLabel(QString("Ouverture : %1").arg(QString::number(lensRadiusSlider->value() / 1000.0, 'f', 3)));
    globalLayout->addRow(lensRadiusLabel, lensRadiusSlider);
    connect(lensRadiusSlider, &QSlider::valueChanged, this, [this, lensRadiusLabel](int value){
        float v = value / 1000.0f;
        lensRadiusLabel->setText(QString("Ouverture : %1").arg(QString::number(v, 'f', 3)));
        m_glWindow->setLensRadius(v);
    });

    QSlider *focalDistanceSlider = new QSlider(Qt::Horizontal);
    focalDistanceSlider->setRange(1, 200); 
    focalDistanceSlider->setValue(50); 
    QLabel *focalDistanceLabel = new QLabel(QString("Distance focal : %1").arg(QString::number(focalDistanceSlider->value() / 10.0, 'f', 2)));
    globalLayout->addRow(focalDistanceLabel, focalDistanceSlider);
    connect(focalDistanceSlider, &QSlider::valueChanged, this, [this, focalDistanceLabel](int value){
        float v = value / 10.0f;
        focalDistanceLabel->setText(QString("Distance focal : %1").arg(QString::number(v, 'f', 2)));
        m_glWindow->setFocalDistance(v);
    });

    connect(dofCheckbox, &QCheckBox::toggled, m_glWindow, &OpenGLWindow::setDoFEnabled);

    QPushButton *resetBtn = new QPushButton("Reset scene");
    globalLayout->addRow(resetBtn);

    globalGroup->setLayout(globalLayout);
    mainLayout->addWidget(globalGroup);

    mainLayout->addStretch();

    scrollArea->setWidget(dockContent);
    dock->setWidget(scrollArea);
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


    connect(xLightSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setLightX);
    connect(yLightSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setLightY);
    connect(zLightSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setLightZ);
    connect(lightIntensitySlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setIntensity);
    connect(lighRadiusSlider, &QSlider::valueChanged, m_glWindow, &OpenGLWindow::setRadius);

    connect(resetBtn, &QPushButton::clicked, this, &mainWindow::on_resetButton_clicked);
}

void mainWindow::openOffMesh()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setNameFilter("Fichiers OFF (*.off)");
    dialog.setWindowTitle("Choisir un mesh");

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().first();

    QtConcurrent::run([this, fileName]() {
        Material mat;
        mat.color = QVector3D(0.8f, 0.8f, 0.8f);
        mat.specularColor = QVector3D(0.0f, 0.0f, 0.0f);
        mat.kd = 1.0f;
        mat.ks = 0.0f;
        mat.shininess = 1.0f;
        mat.type = 0;

        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        m_glWindow->loadOffFile(fileName, verts, idx, mat);

        QMetaObject::invokeMethod(this, [=]() {
            m_glWindow->openOffMesh(fileName, verts, idx, mat);
        });
    });
}

void mainWindow::openObjMesh()
{
    QFileDialog dialog(this);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setNameFilter("Fichiers OBJ (*.obj)");
    dialog.setWindowTitle("Choisir un mesh");

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString fileName = dialog.selectedFiles().first();


    QtConcurrent::run([this, fileName]() {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;
        int faceCount;
        Material mat;
        mat.color = QVector3D(0.8f, 0.8f, 0.8f);
        mat.specularColor = QVector3D(0.0f, 0.0f, 0.0f);
        mat.kd = 1.0f;
        mat.ks = 0.0f;
        mat.shininess = 1.0f;
        mat.type = 0;

        m_glWindow->scene()->loadObjFile(fileName, verts, idx, faceCount,mat);

        QMetaObject::invokeMethod(m_glWindow, [=]() {
            m_glWindow->openOBJmesh(fileName, verts, idx, faceCount, mat);
        });
    });
}

void mainWindow::on_resetButton_clicked()
{
    m_glWindow->resetScene();
    
    Scene* scene = m_glWindow->scene();
    if (!scene) return;

    const QVector<Mesh*>& meshes = scene->meshes();

    if (!meshes.isEmpty() && m_glWindow->m_selectedMesh >= 0 && m_glWindow->m_selectedMesh < meshes.size()) {
        xSlider->setValue(meshes[m_glWindow->m_selectedMesh]->position.x() * 10);
        ySlider->setValue(meshes[m_glWindow->m_selectedMesh]->position.y() * 10);
        zSlider->setValue(meshes[m_glWindow->m_selectedMesh]->position.z() * 10);

        rotationXslider->setValue(meshes[m_glWindow->m_selectedMesh]->rotation.x());
        rotationYslider->setValue(meshes[m_glWindow->m_selectedMesh]->rotation.y());
        rotationZslider->setValue(meshes[m_glWindow->m_selectedMesh]->rotation.z());

        scaleSlider->setValue(meshes[m_glWindow->m_selectedMesh]->scale * 10);

        kdSlider->setValue(meshes[m_glWindow->m_selectedMesh]->material().kd * 100);
        ksSlider->setValue(meshes[m_glWindow->m_selectedMesh]->material().ks * 100);

        shininessSlider->setValue(meshes[m_glWindow->m_selectedMesh]->material().shininess);
        
        typeMat->setCurrentIndex(meshes[m_glWindow->m_selectedMesh]->material().type);
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

    const QVector<Light>& lights = scene->lights();

    if (!lights.isEmpty() && m_glWindow->m_selectedLight >= 0 && m_glWindow->m_selectedLight < lights.size()) {
        xLightSlider->setValue(lights[m_glWindow->m_selectedLight].position.x() * 10);
        yLightSlider->setValue(lights[m_glWindow->m_selectedLight].position.y() * 10);
        zLightSlider->setValue(lights[m_glWindow->m_selectedLight].position.z() * 10);

        lightIntensitySlider->setValue(lights[m_glWindow->m_selectedLight].intensity * 10);
        lighRadiusSlider->setValue(lights[m_glWindow->m_selectedLight].lightRadius * 10);

    }

    lightSelector->clear();

    for (int i = 0; i < lights.size(); ++i) {
        lightSelector->addItem(QString("Light %1").arg(i));
    }
}

void mainWindow::onMeshSelected(int index, const QVector3D &pos, const QVector3D &rota, const float &scale, Material mat)
{
    QSignalBlocker b1(xSlider);
    QSignalBlocker b2(ySlider);
    QSignalBlocker b3(zSlider);
    QSignalBlocker b4(typeMat);
    QSignalBlocker b5(rotationXslider);
    QSignalBlocker b6(rotationYslider);
    QSignalBlocker b7(rotationZslider);
    QSignalBlocker b8(scaleSlider);
    QSignalBlocker b9(kdSlider);
    QSignalBlocker b10(ksSlider);
    QSignalBlocker b11(shininessSlider);
    QSignalBlocker b12(emissionStrengthSlider);
    QSignalBlocker b13(iorSlider);

    xSlider->setValue(pos.x() * 10);
    ySlider->setValue(pos.y() * 10);
    zSlider->setValue(pos.z() * 10);

    xLabel->setText(QString::asprintf("Translation X : %5.1f", pos.x()));
    yLabel->setText(QString::asprintf("Translation Y : %5.1f", pos.y()));
    zLabel->setText(QString::asprintf("Translation Z : %5.1f", pos.z()));

    rotationXslider->setValue(rota.x());
    rotationYslider->setValue(rota.y());
    rotationZslider->setValue(rota.z());

    rotXLabel->setText(QString("Rotation X : %1").arg(rota.x()));
    rotYLabel->setText(QString("Rotation Y : %1").arg(rota.y()));
    rotZLabel->setText(QString("Rotation Z : %1").arg(rota.z()));

    scaleSlider->setValue(scale * 10.0);
    scaleLabel->setText(QString::asprintf("Scale : %5.1f", scale));

    kdSlider->setValue(mat.kd *100);
    ksSlider->setValue(mat.ks *100);

    kdLabel->setText(QString::asprintf("Diffuse (kd): %5.2f", mat.kd));
    ksLabel->setText(QString::asprintf("Spéculaire (ks): %5.2f", mat.ks));

    shininessSlider->setValue(mat.shininess);
    shininessLabel->setText(QString("Brillance : %1").arg(mat.shininess));

    emissionStrengthSlider->setValue(mat.emissionStrength * 10.0);
    emissionStrengthLabel->setText(QString::asprintf("Intensité émission : %5.1f", mat.emissionStrength));
    
    iorSlider->setValue(mat.ior * 100);
    iorLabel->setText(QString("Indice réfraction (verre) : %1").arg(QString::number(mat.ior, 'f', 2)));

    typeMat->setCurrentIndex(mat.type);
}

void mainWindow::onLighthSelected(int index, QVector3D pos, float intensity, float radius)
{
    QSignalBlocker b1(xLightSlider);
    QSignalBlocker b2(yLightSlider);
    QSignalBlocker b3(zLightSlider);
    QSignalBlocker b4(lightIntensitySlider);
    QSignalBlocker b5(lighRadiusSlider);

    xLightSlider->setValue(pos.x() * 10);
    yLightSlider->setValue(pos.y() * 10);
    zLightSlider->setValue(pos.z() * 10);

    lightXLabel->setText(QString::asprintf("Lumière Translation X : %5.1f", pos.x()));
    lightYLabel->setText(QString::asprintf("Lumière Translation Y : %5.1f", pos.y()));
    lightZLabel->setText(QString::asprintf("Lumière Translation Z : %5.1f", pos.z()));

    lightIntensitySlider->setValue(intensity * 10);
    lighRadiusSlider->setValue(radius *10);

    intensityLabel->setText(QString::asprintf("Intensitée : %5.1f", intensity));
    radiusLabel->setText(QString::asprintf("Rayon lumière : %5.1f", radius));
}

void mainWindow::updateFps(float fps)
{
    fpsLabel->setText(QString::number(fps, 'f', 2));
}

void mainWindow::addSphereInScene()
{
    QVector<Mesh::Vertex> verts;
    QVector<unsigned int> idx;
    Material mat;
    mat.color = QVector3D(0.8f, 0.8f, 0.8f);
    mat.specularColor = QVector3D(0.0f, 0.0f, 0.0f);
    mat.kd = 1.0f;
    mat.ks = 0.0f;
    mat.shininess = 1.0f;
    mat.type = 0;

    m_glWindow->scene()->generateSphereMesh(1.0f,20,20,verts,idx,mat);
    m_glWindow->addSphere(verts,idx, mat);
}

void mainWindow::addPlaneInScene()
{
    QVector<Mesh::Vertex> verts;
    QVector<unsigned int> idx;
    Material mat;
    mat.color = QVector3D(0.8f, 0.8f, 0.8f);
    mat.specularColor = QVector3D(0.0f, 0.0f, 0.0f);
    mat.kd = 1.0f;
    mat.ks = 0.0f;
    mat.shininess = 1.0f;
    mat.type = 0;

    m_glWindow->scene()->GenerateQuad({-3,0.,-3},{-3,0.,3},{3,0.,3},{3,0.,-3},{1.0f, 1.0f, 1.0f},verts,idx);
    m_glWindow->addPlane(verts,idx, mat);
}

void mainWindow::addLight(){
    Light l;
    l.position = QVector3D(2.0f, 4.0f, 2.0f);
    l.color    = QVector3D(1.0f, 1.f, 1.f);
    l.intensity= 25.2f;
    l.lightRadius = 1.1f;
    m_glWindow->scene()->addLight(l);
    m_glWindow->updateLightList();
}
