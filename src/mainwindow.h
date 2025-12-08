#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QComboBox>
#include <QLabel>

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
    void onMeshSelected(int index, const QVector3D &pos, const QVector3D &rota, const float &scale);
    void updateFps(float fps);


private:
    OpenGLWindow *m_glWindow;
    QComboBox *meshSelector;
    void on_resetButton_clicked();

    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;

    QSlider *rotationXslider;
    QSlider *rotationYslider;
    QSlider *rotationZslider;

    QSlider *scaleSlider;
    QLabel *fpsLabel;

    QPalette *palette;

signals :


};

#endif // MAINWINDOW_H
