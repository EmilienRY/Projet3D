#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include "scene/material.h"
#include <QGroupBox>
class OpenGLWindow;

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    mainWindow(QWidget *parent = nullptr);
    virtual ~mainWindow() = default;

private slots:
    void openOffMesh();
    void openObjMesh();
    void onMeshSelected(int index, const QVector3D &pos, const QVector3D &rota, const float &scale, Material mat);
    void onLighthSelected(int index, QVector3D pos, float intensity, float radius);
    void updateFps(float fps);
    void addSphereInScene();
    void addPlaneInScene();
    void addLight();


private:
    OpenGLWindow *m_glWindow;
    QComboBox *meshSelector;
    QComboBox *typeMat;
    QComboBox *lightSelector;
    void on_resetButton_clicked();

    void setupMenus();
    void setupDock();
    QGroupBox* createMeshGroup();
    QGroupBox* createMaterialGroup();
    QGroupBox* createLightGroup();
    QGroupBox* createGlobalGroup();

    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;

    QSlider *rotationXslider;
    QSlider *rotationYslider;
    QSlider *rotationZslider;

    QSlider *ksSlider;
    QSlider *kdSlider;
    QSlider *shininessSlider;
    QSlider *emissionStrengthSlider;
    QSlider *iorSlider;

    QSlider *scaleSlider;

    QSlider *xLightSlider;
    QSlider *yLightSlider;
    QSlider *zLightSlider;

    QLabel *fpsLabel;

    QSlider *lightIntensitySlider;
    QSlider *lighRadiusSlider;

    QLabel *xLabel;
    QLabel *yLabel;
    QLabel *zLabel;
    QLabel *rotXLabel;
    QLabel *rotYLabel;
    QLabel *rotZLabel;
    QLabel *scaleLabel;
    QLabel *ksLabel;
    QLabel *kdLabel;
    QLabel *shininessLabel;
    QLabel *emissionStrengthLabel;
    QLabel *iorLabel;
    QLabel *lightXLabel;
    QLabel *lightYLabel;
    QLabel *lightZLabel;
    QLabel *intensityLabel;
    QLabel *radiusLabel;
};

#endif
