#include <QCoreApplication>
#include "openglwindow.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFile>
#include <QDebug>
#include <QMenu>
#include <QDir>
#include <QDateTime>
#include "scene/mesh.h"
#include "scene/scene.h"
#include "gpu_stucts.h"


OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QOpenGLWindow(QOpenGLWindow::NoPartialUpdate, parent)
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
    uploadSceneToGPU();
    resetAccumulation();
    doneCurrent();
    update();
}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "OpenGL Version:" << (const char*)glGetString(GL_VERSION);
    qDebug() << "GLSL Version:"  << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    qDebug() << "Renderer:"      << (const char*)glGetString(GL_RENDERER);
    qDebug() << "Vendor:"        << (const char*)glGetString(GL_VENDOR);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    glBindVertexArray(0);

    glGenTextures(1, &m_currentTex);
    glBindTexture(GL_TEXTURE_2D, m_currentTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1, width()), qMax(1, height()), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_denoisedTex);
    glBindTexture(GL_TEXTURE_2D, m_denoisedTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_accumTex);
    glBindTexture(GL_TEXTURE_2D, m_accumTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1, width()), qMax(1, height()), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_gBufferTex);
    glBindTexture(GL_TEXTURE_2D, m_gBufferTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(), 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_lastCamPos = m_camera.position();
    m_lastCamFront = m_camera.front();
    m_lastCamUp = m_camera.up();
    m_accumFrame = 0;

    m_sceneIndex = 0;
    m_scene->buildPlaneSphere();

    glGetTextureHandleARB = (PFNGLGETTEXTUREHANDLEARBPROC)context()->getProcAddress("glGetTextureHandleARB");
    glMakeTextureHandleResidentARB = (PFNGLMAKETEXTUREHANDLERESIDENTARBPROC)context()->getProcAddress("glMakeTextureHandleResidentARB");

    if (!glGetTextureHandleARB || !glMakeTextureHandleResidentARB) {
        qWarning() << "GL_ARB_bindless_texture not supported or not found!";
    }

    loadShaders();
    uploadSceneToGPU();

    m_frameTimer.start();
    m_lastTimeMs = m_frameTimer.elapsed();
    m_lastFpsTime = m_lastTimeMs;
    m_camera.setPosition(QVector3D(0.0f, 1.5f, 5.0f));
    m_camera.setYawPitch(-90.0f, -10.0f);

    emit sceneReady();
}

void OpenGLWindow::resizeGL(int w, int h)
{
    if (h == 0) h = 1;
    float aspect = float(w) / float(h);
    m_camera.setPerspective(60.0f, aspect, 0.1f, 100.0f);
    glViewport(0, 0, w, h);
    glBindTexture(GL_TEXTURE_2D, m_currentTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1,w), qMax(1,h), 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (m_denoisedTex) {
        glBindTexture(GL_TEXTURE_2D, m_denoisedTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1,w), qMax(1,h), 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (m_accumTex) {
        glBindTexture(GL_TEXTURE_2D, m_accumTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1,w), qMax(1,h), 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
        m_accumFrame = 0;
    }

    if (m_gBufferTex) {
        glBindTexture(GL_TEXTURE_2D, m_gBufferTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1,w), qMax(1,h), 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void OpenGLWindow::resetAccumulation()
{
    m_accumFrame = 0;

    if (m_accumTex) {
        glBindTexture(GL_TEXTURE_2D, m_accumTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, qMax(1,width()), qMax(1,height()), 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void OpenGLWindow::uploadSceneToGPU()
{
    initTexSSBO();

    std::vector<GpuSphere> spheres;
    std::vector<GpuSquare> squares;
    std::vector<GpuLight>  lights;
    std::vector<GpuMesh>  meshes;

    std::vector<GpuVertex> gpuVertices;
    std::vector<GpuTriangleIndexed> gpuIndices;
    std::vector<GpuMaterial> gpuMaterials;

    std::vector<Mesh*> triangleMeshes;

    for (Mesh* mesh : m_scene->meshes())
    {
        if (mesh->isSphere)
        {
            GpuSphere s;
            QVector3D pos = mesh->position;
            s.cx = pos.x(); s.cy = pos.y(); s.cz = pos.z();
            s.radius = mesh->scale;

            s.diffuseR = mesh->material().color.x();
            s.diffuseG = mesh->material().color.y();
            s.diffuseB = mesh->material().color.z();
            s.kd = mesh->material().kd;
            s.ks = mesh->material().ks;
            s.specularR = mesh->material().specularColor.x();
            s.specularG = mesh->material().specularColor.y();
            s.specularB = mesh->material().specularColor.z();
            s.shininess=mesh->material().shininess;

            s.pad1=0.0f;
            s.pad2=0.0f;
            s.pad3=0.0f;
            s.pad5=0;
            s.pad6=0;
            s.Material_type=mesh->material().type;
            
            s.textureIdx = -1;
            if (!mesh->material().texturePath.isEmpty() && m_textureMap.contains(mesh->material().texturePath)) {
                s.textureIdx = m_textureMap[mesh->material().texturePath];
            }

            spheres.push_back(s);
        }
        else if (mesh->isSquare)
        {
            GpuSquare sq;

            QVector3D A = mesh->modelMatrix.map(mesh->m_Vertices[0].pos);
            QVector3D B = mesh->modelMatrix.map(mesh->m_Vertices[1].pos);
            QVector3D C = mesh->modelMatrix.map(mesh->m_Vertices[2].pos);
            QVector3D D = mesh->modelMatrix.map(mesh->m_Vertices[3].pos);

            sq.ax = A.x(); sq.ay = A.y(); sq.az = A.z(); sq.padA=0.0f;
            sq.bx = B.x(); sq.by = B.y(); sq.bz = B.z(); sq.padB=0.0f;
            sq.cx = C.x(); sq.cy = C.y(); sq.cz = C.z(); sq.padC=0.0f;
            sq.dx = D.x(); sq.dy = D.y(); sq.dz = D.z(); sq.padD=0.0f;

            sq.diffuseR = mesh->material().color.x();
            sq.diffuseG = mesh->material().color.y();
            sq.diffuseB = mesh->material().color.z();
            sq.kd = mesh->material().kd;
            sq.ks = mesh->material().ks;
            sq.specularR = mesh->material().specularColor.x();
            sq.specularG = mesh->material().specularColor.y();
            sq.specularB = mesh->material().specularColor.z();
            sq.shininess=mesh->material().shininess;
            sq.Material_type=mesh->material().type;

            sq.textureIdx = -1;
            if (!mesh->material().texturePath.isEmpty() && m_textureMap.contains(mesh->material().texturePath)) {
                sq.textureIdx = m_textureMap[mesh->material().texturePath];
            }

            sq.pad1=0.0f;
            sq.pad2=0.0f;
            sq.pad3=0.0f;
            sq.pad5=0;
            sq.pad6=0;

            squares.push_back(sq);
        }
        else{
            triangleMeshes.push_back(mesh);
        }
    }

    m_bvh.build(triangleMeshes);
    const auto& bvhNodes = m_bvh.getGPUNodes();
    const auto& sortedTriangles = m_bvh.getTriangles();
    const auto& bvhVertices = m_bvh.getVertices();
    const auto& bvhMaterials = m_bvh.getMaterials();

    for (const auto& v : bvhVertices) {
        GpuVertex gv;
        gv.px = v.pos.x(); gv.py = v.pos.y(); gv.pz = v.pos.z(); gv.pad0 = 0;
        gv.nx = v.normal.x(); gv.ny = v.normal.y(); gv.nz = v.normal.z(); gv.pad1 = 0;
        gv.u=v.uv.x(); gv.v=v.uv.y();  gv.texHandle = 0; gv.pad2 = 0;

        gpuVertices.push_back(gv);
    }

    for (const auto& m : bvhMaterials) {
        GpuMaterial gm;
        gm.diffuseR = m.diffuse.x(); gm.diffuseG = m.diffuse.y(); gm.diffuseB = m.diffuse.z();
        gm.kd = m.kd;
        gm.specularR = m.specular.x(); gm.specularG = m.specular.y(); gm.specularB = m.specular.z();
        gm.ks = m.ks;
        gm.shininess = m.shininess;
        gm.type = m.type;
        gm.pad1=0; gm.pad2=0;
        
        gm.textureIdx = -1;
        if (!m.texturePath.isEmpty() && m_textureMap.contains(m.texturePath)) {
            gm.textureIdx = m_textureMap[m.texturePath];
        }

        gm.pad5=0; gm.pad6=0; gm.pad7=0;
        gpuMaterials.push_back(gm);
    }

    for (const auto& tri : sortedTriangles) {
        GpuTriangleIndexed gti;
        gti.v0 = tri.idx0;
        gti.v1 = tri.idx1;
        gti.v2 = tri.idx2;
        gti.matIdx = tri.matIdx;
        gpuIndices.push_back(gti);
    }

    for (auto &l : m_scene->lights())
    {
        GpuLight g;
        g.px = l.position.x();
        g.py = l.position.y();
        g.pz = l.position.z();
        g.intensity = l.intensity;

        g.r = l.color.x();
        g.g = l.color.y();
        g.b = l.color.z();
        g.pad0 = 0.0f;
        g.lightRadius = l.lightRadius;
        g.pad1 = 0.0f;
        g.pad2 = 0.0f;
        g.pad3 = 0.0f;
        lights.push_back(g);
    }

    m_gpuSphereCount = spheres.size();
    m_gpuSquareCount = squares.size();
    m_gpuLightCount  = lights.size();
    m_gpuTriangleCount = gpuIndices.size();
    m_gpuMeshCount = meshes.size();

    //envoie des spheres
    if (!m_ssboSpheres) glGenBuffers(1, &m_ssboSpheres);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboSpheres);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuSphere)*spheres.size(),
                 spheres.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssboSpheres);

    //envoie lumières
    if (!m_ssboLights) glGenBuffers(1, &m_ssboLights);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboLights);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuLight)*lights.size(),
                 lights.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_ssboLights);

    //envoie square
    if (!m_squaresSSBO) glGenBuffers(1, &m_squaresSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_squaresSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuSquare)*squares.size(),
                 squares.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_squaresSSBO);

    //envoie mesh (faut voir pour le remove)
    if (!m_ssboMesh) glGenBuffers(1, &m_ssboMesh);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMesh);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuMesh)*meshes.size(),
                 meshes.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_ssboMesh);

    // envoie des indices des sommets des triangles
    if (!m_ssboIndices) glGenBuffers(1, &m_ssboIndices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboIndices);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuTriangleIndexed)*gpuIndices.size(),
                 gpuIndices.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_ssboIndices);

    // envoie des noeuds de BVH
    if (!m_ssboBVHNodes) glGenBuffers(1, &m_ssboBVHNodes);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboBVHNodes);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GPUBVHNode)*bvhNodes.size(),
                 bvhNodes.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_ssboBVHNodes);

    // envoie vertices
    if (!m_ssboVertices) glGenBuffers(1, &m_ssboVertices);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboVertices);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuVertex)*gpuVertices.size(),
                 gpuVertices.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_ssboVertices);

    // envoie materials
    if (!m_ssboMaterials) glGenBuffers(1, &m_ssboMaterials);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboMaterials);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GpuMaterial)*gpuMaterials.size(),
                 gpuMaterials.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, m_ssboMaterials);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


void OpenGLWindow::initTexSSBO()
{
    m_textureHandles.clear();
    m_textureMap.clear();

    for (Mesh* mesh : m_scene->meshes())
    {
        QString path = mesh->material().texturePath;
        if (path.isEmpty()) continue;
        if (m_textureMap.contains(path)) continue;

        QImage img(path);
        if (img.isNull()) {
            qWarning() << "Failed to load texture:" << path;
            continue;
        }
        img = img.convertToFormat(QImage::Format_RGBA8888);
        img = img.mirrored();

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        const GLuint64 handle = glGetTextureHandleARB(texture);
        if (handle == 0) {
            qDebug() << "Error! Handle returned null";
            continue;
        }
        glMakeTextureHandleResidentARB(handle);

        m_textureHandles.push_back(handle);
        m_textureMap[path] = m_textureHandles.size() - 1;
    }

    if (!m_textureHandles.empty()) {
        if (!m_ssboTextuesHandles) glGenBuffers(1, &m_ssboTextuesHandles);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssboTextuesHandles);
        glBufferData(GL_SHADER_STORAGE_BUFFER, m_textureHandles.size() * sizeof(GLuint64),
                     m_textureHandles.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, m_ssboTextuesHandles);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void OpenGLWindow::doRayTrace()
{
    qint64 now = m_frameTimer.elapsed();
    float dt = (now - m_lastTimeMs) / 1000.0f;
    m_lastTimeMs = now;

    QVector3D dir = inputDirection();
    QVector3D worldMove(0,0,0);
    if (!dir.isNull()) {
        QVector3D forward = m_camera.front();
        QVector3D right   = QVector3D::crossProduct(forward, {0,1,0}).normalized();
        worldMove = forward * dir.z() + right * dir.x();
    }
    m_camera.processKeyboard(worldMove, dt);

    if ((m_camera.position() - m_lastCamPos).length() > 1e-4f ||
        (m_camera.front() - m_lastCamFront).length() > 1e-4f ||
        (m_camera.up() - m_lastCamUp).length() > 1e-4f)
    {
        resetAccumulation();
        m_lastCamPos   = m_camera.position();
        m_lastCamFront = m_camera.front();
        m_lastCamUp    = m_camera.up();
    }

    int gx = (width()+15)/16;
    int gy = (height()+15)/16;

    // RAYTRACE
    m_computeProgram->bind();

    m_computeProgram->setUniformValue("u_sphereCount",  m_gpuSphereCount);
    m_computeProgram->setUniformValue("u_lightCount",   m_gpuLightCount);
    m_computeProgram->setUniformValue("u_squareCount",  m_gpuSquareCount);
    m_computeProgram->setUniformValue("u_camPos",       m_camera.position());
    m_computeProgram->setUniformValue("u_camFront",     m_camera.front());
    m_computeProgram->setUniformValue("u_camRight",     m_camera.right());
    m_computeProgram->setUniformValue("u_camUp",        m_camera.up());
    m_computeProgram->setUniformValue("u_fovDeg",       60.0f);
    m_computeProgram->setUniformValue("u_width",        width());
    m_computeProgram->setUniformValue("u_height",       height());
    m_computeProgram->setUniformValue("u_frameIndex",   m_accumFrame);
    m_computeProgram->setUniformValue("u_triangleCount",  m_gpuTriangleCount);
    m_computeProgram->setUniformValue("u_meshCount",  m_gpuMeshCount);
    m_computeProgram->setUniformValue("u_maxBounces",  m_maxBounces);
    m_computeProgram->setUniformValue("u_shadowSamples",  m_shadowSamples);

    int currentSpp  = 1;
    m_computeProgram->setUniformValue("u_spp", currentSpp);

    glBindImageTexture(0, m_currentTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(4, m_gBufferTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(gx, gy, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    m_computeProgram->release();


    // DENOISE + ACCUMULATION

    m_denoiseProgram->bind();
    m_denoiseProgram->setUniformValue("u_frameIndex", m_accumFrame);

    int passes = 3;

    for (int i = 0; i < passes; i++)
    {
        int step = 1 << i;
        m_denoiseProgram->setUniformValue("u_stepSize", step);
        m_denoiseProgram->setUniformValue("u_passIndex", i);

        glBindImageTexture(0, m_currentTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

        glBindImageTexture(2, m_accumTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        glBindImageTexture(3, m_gBufferTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

        if (i == 0)
        {
            glBindImageTexture(1, m_denoisedTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        }
        else if (i == 1)
        {
            glBindImageTexture(5, m_denoisedTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_currentTex,  0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        }
        else if (i == 2)
        {
            glBindImageTexture(5, m_currentTex,  0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, m_denoisedTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        }

        glDispatchCompute(gx, gy, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    m_denoiseProgram->release();

    // AFFICHAGE

    glDisable(GL_DEPTH_TEST);

    m_screenProgram->bind();
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, m_denoisedTex);
    m_screenProgram->setUniformValue("tex", 0);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    m_screenProgram->release();

    if ((m_accumFrame & 31) == 0) {
        qDebug() << "FrameIndex(accum)=" << m_accumFrame;
    }

    m_accumFrame++;
    update();
}



void OpenGLWindow::screenshot()
{
    glBindTexture(GL_TEXTURE_2D, m_denoisedTex);
    int w = width();
    int h = height();
    std::vector<float> pixels(w * h * 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    QImage img(w, h, QImage::Format_RGBA8888);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w + x) * 4;
            float r = pixels[idx + 0];
            float g = pixels[idx + 1];
            float b = pixels[idx + 2];
            float a = pixels[idx + 3];
            r = qMin(qMax(r, 0.0f), 1.0f);
            g = qMin(qMax(g, 0.0f), 1.0f);
            b = qMin(qMax(b, 0.0f), 1.0f);
            a = qMin(qMax(a, 0.0f), 1.0f);
            img.setPixelColor(x, h - y - 1, QColor::fromRgbF(r, g, b, a));
        }
    }

    qDebug() << "Saving offline render...";
        
        

    QDir dir;
    if (!dir.exists("../../screenshots")) {
        dir.mkdir("../../screenshots");
    }
    QString fileName = QString("../../screenshots/screen%1.png").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    if (img.save(fileName)) {
        qDebug() << "Saved to" << fileName;
    } else {
        qWarning() << "Failed to save" << fileName;
    }
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

    glEnable(GL_DEPTH_TEST);
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
    m_frameCount++;
    qint64 now = m_frameTimer.elapsed();
    if (now - m_lastFpsTime >= 1000) {
        float fps = m_frameCount * 1000.0f / (now - m_lastFpsTime);
        emit fpsChanged(fps);
        m_frameCount = 0;
        m_lastFpsTime = now;
    }

    if(m_useRaytracing)
    {
        if ((m_camera.position() - m_lastCamPos).length() > 1e-4f ||
            (m_camera.front() - m_lastCamFront).length() > 1e-4f ||
            (m_camera.up() - m_lastCamUp).length() > 1e-4f)
        {
            uploadSceneToGPU();

        }

        if(!m_offLineMode)
        {
            doRayTrace();
        }

    }
    else
    {
        doRaster();
    }
}
void OpenGLWindow::loadShaders()
{

    // RAYTRACING SHADER

    m_computeProgram = new QOpenGLShaderProgram();
    if (!m_computeProgram->addShaderFromSourceFile(
            QOpenGLShader::Compute, "src/shaders/raytrace.comp")) {
        qWarning() << "Compute shader compile error:" << m_computeProgram->log();
    }
    if (!m_computeProgram->link()) {
        qWarning() << "Compute shader link error:" << m_computeProgram->log();
    }


    // DENOISE SHADER

    m_denoiseProgram = new QOpenGLShaderProgram();
    if (!m_denoiseProgram->addShaderFromSourceFile(
            QOpenGLShader::Compute, "src/shaders/denoise.comp"))
    {
        qWarning() << "Denoise shader compile error:" << m_denoiseProgram->log();
    }
    if (!m_denoiseProgram->link())
    {
        qWarning() << "Denoise shader link error:" << m_denoiseProgram->log();
    }


    // SCREEN SHADER

    m_screenProgram = new QOpenGLShaderProgram();
    if (!m_screenProgram->addShaderFromSourceFile(
            QOpenGLShader::Vertex, "src/shaders/screen.vert"))
        qWarning() << "Screen vertex compile error:" << m_screenProgram->log();

    if (!m_screenProgram->addShaderFromSourceFile(
            QOpenGLShader::Fragment, "src/shaders/screen.frag"))
        qWarning() << "Screen frag compile error:" << m_screenProgram->log();

    if (!m_screenProgram->link())
        qWarning() << "Screen program link error:" << m_screenProgram->log();

    // RASTERIZATION

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
    #if QT_VERSION >= QT_VERSION_CHECK(6,10,0)
        setKeyboardGrabEnabled(false);
        setMouseGrabEnabled(false);
    #else

    #endif
        return;
    }

    if (ev->key() == Qt::Key_W || ev->key() == Qt::Key_Z || ev->key() == Qt::Key_S || ev->key() == Qt::Key_A || ev->key() == Qt::Key_D) {
        resetAccumulation();
    }

    if (ev->key() == Qt::Key_Plus || ev->text() == "+") {
        changeScene();
        emit sceneReady();
    }

    if (ev->key() == Qt::Key_R) {
        m_useRaytracing = !m_useRaytracing;
        resetAccumulation();
        qDebug() << "Raytracing mode =" << m_useRaytracing;
    }

    if (ev->key() == Qt::Key_O) {
        printf("screen");
        screenshot();

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
    #if QT_VERSION >= QT_VERSION_CHECK(6,10,0)
        setKeyboardGrabEnabled(true);
        setMouseGrabEnabled(true);
    #else

    #endif
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
    resetAccumulation();
}

void OpenGLWindow::focusOutEvent(QFocusEvent *ev)
{
    if (m_fpsActive) {
        m_fpsActive = false;
        setCursor(Qt::ArrowCursor);
#if QT_VERSION >= QT_VERSION_CHECK(6,10,0)
    setKeyboardGrabEnabled(false);
    setMouseGrabEnabled(false);
#else

#endif
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

void OpenGLWindow::openOffMesh(const QString filename, const QVector<Mesh::Vertex> &verts, const QVector<unsigned int> &idx)
{
    makeCurrent();
    QString meshName = filename;
    meshName.remove(0,filename.lastIndexOf("/")+1);
    meshName.remove(meshName.indexOf("."), meshName.size());

    Mesh* mesh = new Mesh();
    mesh->initialize(verts, idx);
    mesh->modelMatrix.setToIdentity();

    int countName = 0;

    m_scene->addMesh(mesh);

    for (int i = 0; i < m_scene->meshes().size(); ++i) {
        if (meshName == m_scene->meshes()[i]->name){
            countName++;
        }
    }

    if (countName > 0){
        meshName.insert(meshName.size(), QString::number(countName+1));
    }
    mesh->name = meshName;

    uploadSceneToGPU();
    resetAccumulation();
    doneCurrent();
    update();
    emit sceneReady();
}

void OpenGLWindow::openOBJmesh(const QString filename, const QVector<Mesh::Vertex> &verts, const QVector<unsigned int> &idx, const int &faceCount)
{
    makeCurrent();
    QString meshName = filename;
    meshName.remove(0,filename.lastIndexOf("/")+1);
    meshName.remove(meshName.indexOf("."), meshName.size());

    Mesh* mesh = new Mesh();
    mesh->initialize(verts, idx);
    mesh->nbTriangles=faceCount;
    mesh->modelMatrix.setToIdentity();

    int countName = 0;

    m_scene->addMesh(mesh);

    for (int i = 0; i < m_scene->meshes().size(); ++i) {
        if (meshName == m_scene->meshes()[i]->name){
            countName++;
        }
    }

    if (countName > 0){
        meshName.insert(meshName.size(), QString::number(countName+1));
    }
    mesh->name = meshName;

    uploadSceneToGPU();
    resetAccumulation();
    doneCurrent();
    update();
    emit sceneReady();
}

void OpenGLWindow::setSelectedMesh(int index)
{
    m_selectedMesh = index;

    if (index >= 0 && index < m_scene->meshes().size()) {
        QVector3D pos = m_scene->meshes()[m_selectedMesh]->position;
        QVector3D rota = m_scene->meshes()[m_selectedMesh]->rotation;
        float scale = m_scene->meshes()[m_selectedMesh]->scale;
        emit selectedMeshChanged(index, pos, rota, scale);
    }

    update();
}

void OpenGLWindow::resetScene(){
    m_sceneIndex -=1;
    changeScene();
}

void OpenGLWindow::setAxeX(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    mesh->position.setX(value / 10.0f);

    mesh->updateModelMatrix();

    update();
}

void OpenGLWindow::setAxeY(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    mesh->position.setY(value / 10.0f);

    mesh->updateModelMatrix();

    update();
}

void OpenGLWindow::setAxeZ(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    mesh->position.setZ(value / 10.0f);

    mesh->updateModelMatrix();

    update();
}

void OpenGLWindow::setRotationX(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    mesh->rotation.setX(value);

    mesh->updateModelMatrix();
}

void OpenGLWindow::setRotationY(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    mesh->rotation.setY(value);

    mesh->updateModelMatrix();
}

void OpenGLWindow::setRotationZ(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    mesh->rotation.setZ(value);

    mesh->updateModelMatrix();
}

void OpenGLWindow::setScale(int value)
{
    if (!m_scene) return;

    const QVector<Mesh*>& meshes = m_scene->meshes();
    if (m_selectedMesh < 0 || m_selectedMesh >= meshes.size()) return;

    Mesh* mesh = meshes[m_selectedMesh];
    if (value > 0){
        mesh->scale = (float)value/10.0;
    }
    else {
        mesh->scale = 0.1;
    }

    mesh->updateModelMatrix();
}




