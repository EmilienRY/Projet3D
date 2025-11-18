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
    static void loadOffFile(const QString &fileName,
                     QVector<Mesh::Vertex> &verts,
                     QVector<unsigned int> &idx);
    void changeScene();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;
    void resetAccumulation();
private:

    void doRayTrace();
    void doRaster();

    QStatusBar * statusbar;
    bool m_useRaytracing = false;

    void loadShaders();

    QVector3D inputDirection() const;
    void uploadSceneToGPU();
    QOpenGLShaderProgram *m_program { nullptr };
    Scene *m_scene { nullptr };
    Camera m_camera;
    QElapsedTimer m_frameTimer;
    qint64 m_lastTimeMs {0};
    QSet<int> m_keysPressed;
    int m_sceneIndex = 0;

    bool m_fpsActive { false };
    QPointF m_lastMousePos;

    GLuint m_computeTex = 0;
    QOpenGLShaderProgram* m_computeProgram = nullptr;
    QOpenGLShaderProgram* m_screenProgram  = nullptr;

    GLuint m_ssboSpheres = 0;
    GLuint m_ssboLights  = 0;
    GLuint m_squaresSSBO = 0;

    GLuint m_quadVAO = 0;
    GLuint m_accumTex = 0;
    int m_accumFrame = 0;
    int m_maxBounces = 4;

    QVector3D m_lastCamPos;
    QVector3D m_lastCamFront;
    QVector3D m_lastCamUp;

    int m_gpuSphereCount = 0;
    int m_gpuLightCount = 0;
    int m_gpuSquareCount = 0;




};
