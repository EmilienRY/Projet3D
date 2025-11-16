#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>

class OpenGLWindow;

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    mainWindow(QWidget *parent = nullptr);

private slots:
    void openOffMesh();

private:
    OpenGLWindow *m_glWindow;
};

#endif // MAINWINDOW_H
