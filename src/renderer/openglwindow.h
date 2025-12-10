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
#include "renderer/bvh.h"

class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT
public:
    explicit OpenGLWindow(QWindow *parent = nullptr);
    ~OpenGLWindow();
    void openOffMesh(const QString filename, const QVector<Mesh::Vertex> &verts, const QVector<unsigned int> &idx, Material mat);
    static void loadOffFile(const QString &fileName,
                     QVector<Mesh::Vertex> &verts,
                     QVector<unsigned int> &idx,
                     Material mat);
    void changeScene();
    void resetScene();

    void openOBJmesh(const QString filename, const QVector<Mesh::Vertex> &verts, const QVector<unsigned int> &idx, const int &faceCount, Material mat);

    void setAxeX(int value);
    void setAxeY(int value);
    void setAxeZ(int value);
    void setRotationX(int value);
    void setRotationY(int value);
    void setRotationZ(int value);
    void setScale(int value);
    void setKs(int value);
    void setKd(int value);
    void setShininess(int value);
    void setLightX(int value);
    void setLightY(int value);
    void setLightZ(int value);
    void setIntensity(int value);
    void setRadius(int value);

    Scene* scene() { return m_scene; }
    void setSelectedMesh(int index);
    void setSelectedLight(int index);
    void setSelectedTypeMat(int index);
    int m_selectedMesh = -1;
    int m_selectedLight = -1;
    int m_selectedType = -1;


    void changeColor(QColor color);
    void changeColorSpec(QColor color);
    void changeColorLight(QColor color);

    void addSphere(QVector<Mesh::Vertex> verts, QVector<unsigned int> idx, Material mat);
    void addPlane(QVector<Mesh::Vertex> verts, QVector<unsigned int> idx, Material mat);
    void updateLightList();

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
    typedef GLuint64 (APIENTRY *PFNGLGETTEXTUREHANDLEARBPROC) (GLuint texture);
    typedef void (APIENTRY *PFNGLMAKETEXTUREHANDLERESIDENTARBPROC) (GLuint64 handle);
    
    PFNGLGETTEXTUREHANDLEARBPROC glGetTextureHandleARB = nullptr;
    PFNGLMAKETEXTUREHANDLERESIDENTARBPROC glMakeTextureHandleResidentARB = nullptr;

    std::vector<GLuint64> m_textureHandles;
    QMap<QString, int> m_textureMap;
    
    void doRayTrace();
    void doRaster();
    void initTexSSBO();
    void screenshot();

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
    int m_frameCount {0};
    qint64 m_lastFpsTime {0};
    QSet<int> m_keysPressed;
    int m_sceneIndex = 0;

    bool m_fpsActive { false };
    QPointF m_lastMousePos;

    QOpenGLShaderProgram* m_computeProgram = nullptr;
    QOpenGLShaderProgram* m_screenProgram  = nullptr;
    QOpenGLShaderProgram* m_denoiseProgram  = nullptr;

    GLuint m_ssboSpheres = 0;
    GLuint m_ssboLights  = 0;
    GLuint m_squaresSSBO = 0;
    GLuint m_ssboMesh = 0;
    GLuint m_ssboBVHNodes = 0;
    GLuint m_ssboTextuesHandles = 0;

    GLuint m_ssboVertices = 0;
    GLuint m_ssboIndices = 0;
    GLuint m_ssboMaterials = 0;

    GLuint m_quadVAO = 0;
    GLuint m_accumTex = 0;
    GLuint m_currentTex = 0;
    GLuint m_denoisedTex = 0;
    GLuint m_gBufferTex = 0;

    int m_accumFrame = 0;
    int m_maxBounces = 4;
    int m_shadowSamples = 1;

    bool m_offLineMode = false;


    QVector3D m_lastCamPos;
    QVector3D m_lastCamFront;
    QVector3D m_lastCamUp;

    int m_gpuSphereCount = 0;
    int m_gpuLightCount = 0;
    int m_gpuSquareCount = 0;
    int m_gpuTriangleCount = 0;
    int m_gpuMeshCount = 0;

    BVH m_bvh;

    signals:
        void fpsChanged(float fps);
        void sceneReady();
        void selectedMeshChanged(int index, QVector3D position, QVector3D rota, float scale, Material mat);
        void selectedLightChanged(int index, QVector3D position, float intensity, float radius);
        void selectedTypeChanged(int index);


};
