#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QComboBox>

class OpenGLWindow;

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    mainWindow(QWidget *parent = nullptr);
    virtual ~mainWindow() = default;

private slots:
    void openOffMesh();

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

};

#endif // MAINWINDOW_H
