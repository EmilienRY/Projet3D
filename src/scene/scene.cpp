#include "scene.h"
#include "mesh.h"

Scene::Scene()
{
}

Scene::~Scene()
{
    for (Mesh* m : m_meshes) {
        delete m;
    }
    m_meshes.clear();
}

void Scene::addMesh(Mesh* m)
{
    if (m) m_meshes.append(m);
}

void Scene::clear()
{
    m_meshes.clear();
    m_lights.clear();
}

void generateSphereMesh(float radius,
                        int stacks, int slices,
                        QVector<Mesh::Vertex>& verts,
                        QVector<unsigned int>& idx)
{
    verts.clear();
    idx.clear();

    for (int i = 0; i <= stacks; ++i) {
        float v = float(i) / stacks;
        float phi = v * M_PI;

        for (int j = 0; j <= slices; ++j) {
            float u = float(j) / slices;
            float theta = u * 2.0f * M_PI;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            verts.append({ QVector3D(x, y, z),
                          QVector3D(1,1,1) });
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int a =  i    * (slices+1) + j;
            int b =  i    * (slices+1) + j+1;
            int c = (i+1) * (slices+1) + j;
            int d = (i+1) * (slices+1) + j+1;

            idx.append(a);
            idx.append(c);
            idx.append(b);

            idx.append(b);
            idx.append(c);
            idx.append(d);
        }
    }
}

void Scene::buildPlaneSphere()
{
    QVector<Mesh::Vertex> verts;
    QVector<unsigned int> idx;

    auto pushQuad = [&](QVector3D a, QVector3D b, QVector3D c, QVector3D d, QVector3D color) {
        unsigned base = verts.size();
        verts.append({a, color});
        verts.append({b, color});
        verts.append({c, color});
        verts.append({d, color});
        idx.append(base+0); idx.append(base+1); idx.append(base+2);
        idx.append(base+2); idx.append(base+3); idx.append(base+0);
    };

    pushQuad(
        {-3,0.,-3},
        {-3,0.,3},
        {3,0.,3},
        {3,0.,-3},
        {0.7f, 0.7f, 0.7f}
    );

    Material m1;
    m1.color=QVector3D(0,1,0);

    Mesh* plane = new Mesh();
    plane->addMaterial(m1);
    plane->initialize(verts, idx);
    addMesh(plane);

    QVector<Mesh::Vertex> sVerts;
    QVector<unsigned int> sIdx;

    generateSphereMesh(1.0f, 20, 20, sVerts, sIdx);

    Material m2;
    m2.color=QVector3D(0,0,1);

    Mesh* sphere = new Mesh();
    sphere->addMaterial(m2);
    sphere->initialize(sVerts, sIdx);
    sphere->modelMatrix.translate(0, 1, 0);
    sphere->isSphere=true;
    addMesh(sphere);

    Light l;
    l.position = QVector3D(2.0f, 4.0f, 2.0f);
    l.color    = QVector3D(1.0f, 1.0f, 1.0f);
    l.intensity= 3.2f;
    m_lights.append(l);

}

void Scene::buildCornellBox()
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

    addMesh(cube);

    Light l;
    l.position=QVector3D(0,1,0);
    l.color=QVector3D(1,1,1);
    l.intensity=1.;

    m_lights.append(l);

}
