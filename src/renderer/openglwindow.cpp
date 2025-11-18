// src/renderer/openglwindow.cpp
#include "openglwindow.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFile>
#include <QDebug>
#include <QMenu>
#include "scene/mesh.h"
#include "scene/scene.h"
#include "gpu_stucts.h"
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

void OpenGLWindow::changeScene()
{
    m_sceneIndex = (m_sceneIndex + 1) % 2;

    makeCurrent();
    if (m_sceneIndex == 0)
    {
        m_scene->clear();
        m_scene->buildPlaneSphere();
    }
    else if (m_sceneIndex == 1)
    {
        m_scene->clear();
        m_scene->buildCornellBox();
    }
    doneCurrent();
    update();
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
    qDebug() << "GLSL Version:"  << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    glBindVertexArray(0);

    glGenTextures(1, &m_computeTex);
    glBindTexture(GL_TEXTURE_2D, m_computeTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1, width()), qMax(1, height()), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_sceneIndex = 0;
    m_scene->buildPlaneSphere();

    loadShaders();

    m_frameTimer.start();
    m_lastTimeMs = m_frameTimer.elapsed();
    m_camera.setPosition(QVector3D(0.0f, 1.5f, 5.0f));
    m_camera.setYawPitch(-90.0f, -10.0f);
}

void OpenGLWindow::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    float aspect = float(w) / float(h);
    m_camera.setPerspective(60.0f, aspect, 0.1f, 100.0f);
    glViewport(0, 0, w, h);
    glBindTexture(GL_TEXTURE_2D, m_computeTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1,w), qMax(1,h), 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void OpenGLWindow::uploadSceneToGPU()
{
    std::vector<GpuSphere> spheres;
    std::vector<GpuLight>  lights;
    std::vector<GpuSquare>  squares;

    for (Mesh* mesh : m_scene->meshes()) {
        if (mesh->isSphere) {
            GpuSphere s;
            auto pos = mesh->modelMatrix.map(QVector3D(0,0,0));
            s.cx = pos.x(); s.cy = pos.y(); s.cz = pos.z();
            s.radius = 1.0f;

            s.r = mesh->material().color.x();
            s.g = mesh->material().color.y();
            s.b = mesh->material().color.z();
            s.pad0 = 0.0f;
            qDebug() << "Upload sphere center" << s.cx << s.cy << s.cz << " color " << s.r << s.g << s.b;
            spheres.push_back(s);
        }
        else {
            // --- SQUARE (quad) ---

            if (mesh->m_Vertices.size() < 4)
                continue;

            GpuSquare sq;

            QVector3D A = mesh->modelMatrix.map(mesh->m_Vertices[0].pos);
            QVector3D B = mesh->modelMatrix.map(mesh->m_Vertices[1].pos);
            QVector3D C = mesh->modelMatrix.map(mesh->m_Vertices[2].pos);
            QVector3D D = mesh->modelMatrix.map(mesh->m_Vertices[3].pos);


            sq.ax = A.x(); sq.ay = A.y(); sq.az = A.z(); sq.pada = 0.0f;
            sq.bx = B.x(); sq.by = B.y(); sq.bz = B.z(); sq.padb = 0.0f;
            sq.cx = C.x(); sq.cy = C.y(); sq.cz = C.z(); sq.padc = 0.0f;
            sq.dx = D.x(); sq.dy = D.y(); sq.dz = D.z(); sq.padd = 0.0f;

            sq.r = mesh->material().color.x();
            sq.g = mesh->material().color.y();
            sq.b = mesh->material().color.z();
            sq.pad0 = 0.0f;

            squares.push_back(sq);
        }
    }

    for (auto &l : m_scene->lights()) {
        GpuLight g;
        g.px = l.position.x();
        g.py = l.position.y();
        g.pz = l.position.z();
        g.intensity = l.intensity;

        g.r = l.color.x();
        g.g = l.color.y();
        g.b = l.color.z();
        g.pad0 = 0.0f;
        lights.push_back(g);
    }

    m_gpuSphereCount = (int)spheres.size();
    m_gpuLightCount  = (int)lights.size();
    m_gpuSquareCount = (int)squares.size();

    if (!m_ssboSpheres)
        glGenBuffers(1, &m_ssboSpheres);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboSpheres);
    if (!spheres.empty())
        glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(GpuSphere), spheres.data(), GL_DYNAMIC_DRAW);
    else
        glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * sizeof(GpuSphere), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboSpheres);

    if (!m_ssboLights)
        glGenBuffers(1, &m_ssboLights);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboLights);
    if (!lights.empty())
        glBufferData(GL_SHADER_STORAGE_BUFFER, lights.size() * sizeof(GpuLight), lights.data(), GL_DYNAMIC_DRAW);
    else
        glBufferData(GL_SHADER_STORAGE_BUFFER, 1 * sizeof(GpuLight), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ssboLights);


    if (!m_squaresSSBO)
        glGenBuffers(1, &m_squaresSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_squaresSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 squares.size() * sizeof(GpuSquare),
                 squares.data(),
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_squaresSSBO);


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void OpenGLWindow::doRayTrace()
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

    uploadSceneToGPU();

    GLint tw=0, th=0;
    glBindTexture(GL_TEXTURE_2D, m_computeTex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &tw);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &th);

    m_computeProgram->bind();

    m_computeProgram->setUniformValue("u_sphereCount", m_gpuSphereCount);
    m_computeProgram->setUniformValue("u_lightCount", m_gpuLightCount);

    m_computeProgram->setUniformValue("u_camPos",   m_camera.position());
    m_computeProgram->setUniformValue("u_camFront", m_camera.front());
    m_computeProgram->setUniformValue("u_camRight", m_camera.right());
    m_computeProgram->setUniformValue("u_camUp",    m_camera.up());

    m_computeProgram->setUniformValue("u_planeY", -0.5f);

    m_computeProgram->setUniformValue("u_fovDeg", 60.0f);
    m_computeProgram->setUniformValue("u_width", width());
    m_computeProgram->setUniformValue("u_height", height());
    m_computeProgram->setUniformValue("u_squareCount", m_gpuSquareCount);

    glBindImageTexture(0, m_computeTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    int gx = (width()  + 15) / 16;
    int gy = (height() + 15) / 16;
    glDispatchCompute(gx, gy, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

    m_computeProgram->release();

    glDisable(GL_DEPTH_TEST);

    m_screenProgram->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_computeTex);
    m_screenProgram->setUniformValue("tex", 0);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_screenProgram->release();
    update();
}

void OpenGLWindow::doRaster()
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

void OpenGLWindow::paintGL()
{
    if(m_useRaytracing)
    {
        doRayTrace();
    }
    else
    {
        doRaster();
    }
}

void OpenGLWindow::loadShaders()
{
    m_computeProgram = new QOpenGLShaderProgram();
    if (!m_computeProgram->addShaderFromSourceFile(QOpenGLShader::Compute, "src/shaders/raytrace.comp")) {
        qWarning() << "Compute shader compile error:" << m_computeProgram->log();
    }
    if (!m_computeProgram->link()) {
        qWarning() << "Compute shader link error:" << m_computeProgram->log();
    }

    m_screenProgram = new QOpenGLShaderProgram();
    if (!m_screenProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,   "src/shaders/screen.vert"))
        qWarning() << "Screen vertex compile error:" << m_screenProgram->log();
    if (!m_screenProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "src/shaders/screen.frag"))
        qWarning() << "Screen frag compile error:" << m_screenProgram->log();
    if (!m_screenProgram->link())
        qWarning() << "Screen program link error:" << m_screenProgram->log();

    m_program = new QOpenGLShaderProgram();
    bool ok = m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "src/shaders/basic.vert");
    if (!ok) qWarning() << "Vertex shader compile error:" << m_program->log();
    ok = m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "src/shaders/basic.frag");
    if (!ok) qWarning() << "Fragment shader compile error:" << m_program->log();

    if (!m_program->link()) {
        qWarning() << "Shader program link error:" << m_program->log();
    }
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

    if (ev->key() == Qt::Key_Plus || ev->text() == "+") {
        changeScene();
    }

    if (ev->key() == Qt::Key_R) {
        m_useRaytracing = !m_useRaytracing;
        qDebug() << "Raytracing mode =" << m_useRaytracing;
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

    if (m_keysPressed.contains(Qt::Key_Z) || m_keysPressed.contains(Qt::Key_W) || m_keysPressed.contains(Qt::Key_Up)) z += 1.0f;
    if (m_keysPressed.contains(Qt::Key_S) || m_keysPressed.contains(Qt::Key_Down)) z -= 1.0f;
    if (m_keysPressed.contains(Qt::Key_D) || m_keysPressed.contains(Qt::Key_Right)) x += 1.0f;
    if (m_keysPressed.contains(Qt::Key_Q) || m_keysPressed.contains(Qt::Key_A) || m_keysPressed.contains(Qt::Key_Left)) x -= 1.0f;

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

    QVector3D defaultColor(1.0f, 1.0f, 1.0f);

    std::vector<QVector3D> positions;
    positions.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i) {
        float x, y, z;
        in >> x >> y >> z;
        positions.emplace_back(x, y, z);
    }

    for (auto &p : positions)
        verts.append({p, defaultColor});

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




