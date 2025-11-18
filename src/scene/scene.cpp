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

    m1.kd = 1.;
    m1.ks = 0.3;
    m1.specularColor = QVector3D(1,1,1);
    m1.shininess = 32;

    Mesh* plane = new Mesh();
    plane->addMaterial(m1);
    plane->initialize(verts, idx);
    addMesh(plane);

    QVector<Mesh::Vertex> sVerts;
    QVector<unsigned int> sIdx;

    generateSphereMesh(1.0f, 20, 20, sVerts, sIdx);

    Material m2;
    m2.color=QVector3D(1.0, 0., 0.);;
    m2.kd = 0.5;
    m2.ks = 0.6;
    //m2.specularColor = m2.color;
    m2.specularColor = QVector3D(1.0,1.0,1.0);
    m2.shininess = 128;


    Mesh* sphere = new Mesh();
    sphere->addMaterial(m2);
    sphere->initialize(sVerts, sIdx);
    sphere->modelMatrix.translate(0, 1, 0);
    sphere->isSphere=true;
    addMesh(sphere);

    Light l;
    l.position = QVector3D(2.0f, 4.0f, 2.0f);
    l.color    = QVector3D(1.0f, 1.f, 1.f);
    l.intensity= 25.2f;
    m_lights.append(l);
}


void Scene::buildCornellBox()
{
    clear();

    float L = 3.0f;
    float H = 3.0f;
    float D = 3.0f;

    QVector3D A(-L,-H, D);
    QVector3D B( L,-H, D);
    QVector3D C( L, H, D);
    QVector3D Dp(-L, H, D);

    QVector3D E(-L,-H,-D);
    QVector3D F( L,-H,-D);
    QVector3D G( L, H,-D);
    QVector3D Hh(-L, H,-D);

    Material white;  white.color = QVector3D(0.78f,0.78f,0.78f);
    white.kd = 0.9f; white.ks = 0.0f;
    white.specularColor = QVector3D(1,1,1); white.shininess = 32;

    Material red;    red.color = QVector3D(0.65f,0.05f,0.05f);
    red.kd = 0.9f; red.ks = 0.0f;
    red.specularColor = QVector3D(1,1,1); red.shininess = 32;

    Material green;  green.color = QVector3D(0.12f,0.55f,0.15f);
    green.kd = 0.9f; green.ks = 0.0f;
    green.specularColor = QVector3D(1,1,1); green.shininess = 32;

    auto makeQuad = [&](QVector3D a, QVector3D b, QVector3D c, QVector3D d, Material mat)
    {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        unsigned int base = 0;

        verts.append({ d, mat.color });
        verts.append({ c, mat.color });
        verts.append({ b, mat.color });
        verts.append({ a, mat.color });

        idx.append(base+0); idx.append(base+1); idx.append(base+2);
        idx.append(base+2); idx.append(base+3); idx.append(base+0);

        Mesh* m = new Mesh();
        m->initialize(verts, idx);
        m->modelMatrix.setToIdentity();
        m->addMaterial(mat);
        addMesh(m);
    };

    makeQuad(A, B, C, Dp, white);    // front
    makeQuad(F, E, Hh, G, white);    // back
    makeQuad(E, A, Dp, Hh, red);     // left
    makeQuad(B, F, G, C, green);     // right
    makeQuad(Dp, C, G, Hh, white);   // ceiling
    makeQuad(E, F, B, A, white);     // floor

    {
        Mesh* s1 = new Mesh();
        s1->isSphere = true;
        s1->modelMatrix.setToIdentity();
        s1->modelMatrix.translate(1.0f, -2.0f, 0.5f);

        Material m;
        m.color = QVector3D(0.9f, 0.2f, 0.2f);
        m.kd = 0.8f;
        m.ks = 0.2f;
        m.specularColor = QVector3D(1,1,1);
        m.shininess = 64;

        s1->addMaterial(m);
        addMesh(s1);
    }

    {
        Mesh* s2 = new Mesh();
        s2->isSphere = true;
        s2->modelMatrix.setToIdentity();
        s2->modelMatrix.translate(-1.0f, -2.f, -1.0f);

        Material m;
        m.color = QVector3D(0.4f, 0.4f, 1.0f);
        m.kd = 0.8f;
        m.ks = 0.1f;
        m.specularColor = QVector3D(1,1,1);
        m.shininess = 32;

        s2->addMaterial(m);
        addMesh(s2);
    }

    Light l;
    l.position  = QVector3D(0, 2.8f, 0);
    l.color     = QVector3D(1,1,1);
    l.intensity = 10.0f;

    m_lights.append(l);
}

