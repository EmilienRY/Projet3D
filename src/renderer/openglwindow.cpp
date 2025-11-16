// src/renderer/openglwindow.cpp
#include "openglwindow.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QFile>
#include <QDebug>
#include <QMenu>

// IMPORTANT: include the full Mesh definition since we use Mesh::Vertex, modelMatrix, new Mesh()
#include "scene/mesh.h"
#include "scene/scene.h"

OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QOpenGLWindow(NoPartialUpdate, parent)
{
    m_scene = new Scene();
}

OpenGLWindow::~OpenGLWindow()
{
    makeCurrent();
    delete m_program;
    delete m_scene;
    doneCurrent();
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    loadShaders();
    buildScene();

    m_frameTimer.start();
    m_lastTimeMs = m_frameTimer.elapsed();
    // initial camera setup
    m_camera.setPosition(QVector3D(0.0f, 0.0f, 5.0f));
}

void OpenGLWindow::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    float aspect = float(w) / float(h);
    m_camera.setPerspective(60.0f, aspect, 0.1f, 100.0f);
    glViewport(0, 0, w, h);
}

void OpenGLWindow::paintGL()
{
    qint64 now = m_frameTimer.elapsed();
    float dt = (now - m_lastTimeMs) / 1000.0f;
    m_lastTimeMs = now;

    QVector3D dir = inputDirection();
    QVector3D worldMove(0.0f, 0.0f, 0.0f);
    if (!dir.isNull()) {
        QVector3D forward = m_camera.front();
        QVector3D right   = QVector3D::crossProduct(forward, QVector3D(0.0f, 1.0f, 0.0f)).normalized();
        worldMove = forward * dir.z() + right * dir.x();
    }
    m_camera.processKeyboard(worldMove, dt);

    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 model;
    model.setToIdentity();

    QMatrix4x4 view = m_camera.viewMatrix();
    QMatrix4x4 proj;
    float aspect = float(width()) / float(height() ? height() : 1);
    proj.perspective(60.0f, aspect, 0.1f, 100.0f);

    if (m_program) {
        m_program->bind();
        m_program->setUniformValue("view", view);
        m_program->setUniformValue("proj", proj);

        for (Mesh* mesh : m_scene->meshes()) {
            m_program->setUniformValue("model", mesh->modelMatrix * model);
            mesh->render();
        }

        m_program->release();
    }

    update();
}

void OpenGLWindow::loadShaders()
{
    m_program = new QOpenGLShaderProgram();
    bool ok = m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "src/shaders/basic.vert");
    if (!ok) qWarning() << "Vertex shader compile error:" << m_program->log();
    ok = m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "src/shaders/basic.frag");
    if (!ok) qWarning() << "Fragment shader compile error:" << m_program->log();

    if (!m_program->link()) {
        qWarning() << "Shader program link error:" << m_program->log();
    }
}

void OpenGLWindow::buildScene()
{
    QVector<Mesh::Vertex> verts;
    QVector<unsigned int> idx;

    QVector<QVector3D> positions = {
        {-1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        {-1.0f,  1.0f, -1.0f}
    };

    QVector3D cFront(1.0f, 0.0f, 0.0f);
    QVector3D cBack(0.0f, 1.0f, 0.0f);
    QVector3D cLeft(0.0f, 0.0f, 1.0f);
    QVector3D cRight(1.0f, 1.0f, 0.0f);
    QVector3D cTop(1.0f, 0.0f, 1.0f);
    QVector3D cBottom(0.0f, 1.0f, 1.0f);

    auto pushQuad = [&](QVector3D a, QVector3D b, QVector3D c, QVector3D d, QVector3D color) {
        unsigned int base = verts.size();
        verts.append({a, color});
        verts.append({b, color});
        verts.append({c, color});
        verts.append({d, color});
        idx.append(base + 0); idx.append(base + 1); idx.append(base + 2);
        idx.append(base + 2); idx.append(base + 3); idx.append(base + 0);
    };

    pushQuad(positions[0], positions[1], positions[2], positions[3], cFront);
    pushQuad(positions[5], positions[4], positions[7], positions[6], cBack);
    pushQuad(positions[4], positions[0], positions[3], positions[7], cLeft);
    pushQuad(positions[1], positions[5], positions[6], positions[2], cRight);
    pushQuad(positions[3], positions[2], positions[6], positions[7], cTop);
    pushQuad(positions[4], positions[5], positions[1], positions[0], cBottom);

    Mesh* cube = new Mesh();
    cube->initialize(verts, idx);
    cube->modelMatrix.setToIdentity();

    m_scene->addMesh(cube);
}

void OpenGLWindow::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Escape && m_fpsActive) {
        m_fpsActive = false;
        setCursor(Qt::ArrowCursor);
        setKeyboardGrabEnabled(false);
        setMouseGrabEnabled(false);
        return;
    }

    m_keysPressed.insert(ev->key());
    QOpenGLWindow::keyPressEvent(ev);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent *ev)
{
    m_keysPressed.remove(ev->key());
    QOpenGLWindow::keyReleaseEvent(ev);
}

QVector3D OpenGLWindow::inputDirection() const
{
    float x = 0.0f;
    float z = 0.0f;

    if (m_keysPressed.contains(Qt::Key_Z) || m_keysPressed.contains(Qt::Key_W)) z += 1.0f;
    if (m_keysPressed.contains(Qt::Key_S)) z -= 1.0f;
    if (m_keysPressed.contains(Qt::Key_D)) x += 1.0f;
    if (m_keysPressed.contains(Qt::Key_Q) || m_keysPressed.contains(Qt::Key_A)) x -= 1.0f;

    return QVector3D(x, 0.0f, z);
}

void OpenGLWindow::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton && !m_fpsActive) {
        m_fpsActive = true;
        m_lastMousePos = ev->position();
        setCursor(Qt::BlankCursor);
        setKeyboardGrabEnabled(true);
        setMouseGrabEnabled(true);
    }


    QOpenGLWindow::mousePressEvent(ev);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *ev)
{
    if (!m_fpsActive) {
        m_lastMousePos = ev->position();
        QOpenGLWindow::mouseMoveEvent(ev);
        return;
    }

    QPointF cur = ev->position();
    QPointF delta = cur - m_lastMousePos;
    m_lastMousePos = cur;

    m_camera.processMouseMovement(delta.x(), -delta.y());
}

void OpenGLWindow::focusOutEvent(QFocusEvent *ev)
{
    if (m_fpsActive) {
        m_fpsActive = false;
        setCursor(Qt::ArrowCursor);
        setKeyboardGrabEnabled(false);
        setMouseGrabEnabled(false);
    }

    QOpenGLWindow::focusOutEvent(ev);
}

void OpenGLWindow::loadOffFile(const QString &fileName,
                               QVector<Mesh::Vertex> &verts,
                               QVector<unsigned int> &idx)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open OFF file:" << fileName;
    }

    QTextStream in(&file);

    // ----- HEADER -----
    QString header;
    in >> header;
    if (header != "OFF") {
        qWarning() << "Invalid OFF file:" << fileName;
    }

    int vertexCount = 0;
    int faceCount = 0;
    int edgeCount = 0;
    in >> vertexCount >> faceCount >> edgeCount;

    if (vertexCount <= 0 || faceCount <= 0) {
        qWarning() << "Invalid mesh size";
    }

    verts.clear();
    idx.clear();

    verts.reserve(vertexCount);
    idx.reserve(faceCount * 3);

    // ----- DEFAULT COLOR -----
    QVector3D defaultColor(1.0f, 1.0f, 1.0f);

    // ----- READ VERTICES -----
    std::vector<QVector3D> positions;
    positions.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i) {
        float x, y, z;
        in >> x >> y >> z;
        positions.emplace_back(x, y, z);
    }

    // Convert all positions into Mesh::Vertex
    for (auto &p : positions)
        verts.append({p, defaultColor});

    // ----- READ FACES -----
    for (int i = 0; i < faceCount; ++i) {
        int n, a, b, c;
        in >> n >> a >> b >> c;

        if (n != 3) {
            qWarning() << "Non triangular face encountered. Only triangles are supported!";
        }

        idx.append(static_cast<unsigned int>(a));
        idx.append(static_cast<unsigned int>(b));
        idx.append(static_cast<unsigned int>(c));
    }
}

void OpenGLWindow::openOffMesh(const QVector<Mesh::Vertex> &verts,
                               const QVector<unsigned int> &idx)
{
    Mesh* mesh = new Mesh();
    mesh->initialize(verts, idx);
    mesh->modelMatrix.setToIdentity();

    m_scene->addMesh(mesh);

}




