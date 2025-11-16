#pragma once
#include <QOpenGLWindow>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QElapsedTimer>
#include <QSet>
#include <QFileDialog>
#include <QStatusBar>
#include "scene/mesh.h"
#include <memory>
#include "renderer/camera.h"
#include "scene/scene.h"

class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT
public:
    explicit OpenGLWindow(QWindow *parent = nullptr);
    ~OpenGLWindow();
    void openOffMesh(const QVector<Mesh::Vertex> &verts,
                     const QVector<unsigned int> &idx);
    void loadOffFile(const QString &fileName,
                     QVector<Mesh::Vertex> &verts,
                     QVector<unsigned int> &idx);

protected:
    // OpenGL
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Input
    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;

private: 
    QStatusBar * statusbar;

    void loadShaders();
    void buildScene();

    QVector3D inputDirection() const;

    QOpenGLShaderProgram *m_program { nullptr };
    Scene *m_scene { nullptr };
    Camera m_camera;
    QElapsedTimer m_frameTimer;
    qint64 m_lastTimeMs {0};
    QSet<int> m_keysPressed;

    bool m_fpsActive { false };
    QPointF m_lastMousePos;
};
