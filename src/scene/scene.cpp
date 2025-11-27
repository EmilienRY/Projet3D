#include "scene.h"
#include "mesh.h"
#include <QFile>

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
    m1.type=0;

    Mesh* plane = new Mesh();
    plane->addMaterial(m1);
    plane->initialize(verts, idx);
    plane->isSquare=true;
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
    m2.type = 1;


    Mesh* sphere = new Mesh();
    sphere->addMaterial(m2);
    sphere->initialize(sVerts, sIdx);
    sphere->modelMatrix.translate(0, 1, 0);
    sphere->isSphere=true;
    addMesh(sphere);

    Mesh* suzanne = new Mesh();
    QVector<Mesh::Vertex> meshVerts;
    QVector<unsigned int> meshIdx;
    int nbTriangle;
    QString meshFile = "../../meshes/suzanne.off";
    loadOffFile(meshFile, meshVerts, meshIdx, nbTriangle);

    suzanne->nbTriangles=nbTriangle;
    suzanne->initialize(meshVerts,meshIdx);
    suzanne->modelMatrix.translate(-1, 1, 2);
    suzanne->modelMatrix.scale(0.5);

    Material cyan;
    cyan.color=QVector3D(0,1,1);

    cyan.kd = 1.;
    cyan.ks = 0.3;
    cyan.specularColor = QVector3D(1,1,1);
    cyan.shininess = 32;
    cyan.type=0;

    suzanne->addMaterial(cyan);
    addMesh(suzanne);

    Mesh* suzanne2 = new Mesh();
    QVector<Mesh::Vertex> meshVerts2;
    QVector<unsigned int> meshIdx2;
    int nbTriangle2;
    QString meshFile2 = "../../meshes/suzanne.off";
    loadOffFile(meshFile2, meshVerts2, meshIdx2, nbTriangle2);

    suzanne2->nbTriangles=nbTriangle2;
    suzanne2->initialize(meshVerts2,meshIdx2);
    suzanne2->modelMatrix.translate(1, 1, 2);
    suzanne2->modelMatrix.scale(0.5);

    Material cyan2;
    cyan2.color=QVector3D(1,0,0);

    cyan2.kd = 0.7;
    cyan2.ks = 0.7;
    cyan2.specularColor = QVector3D(1,1,1);
    cyan2.shininess = 32;
    cyan2.type=0;

    suzanne2->addMaterial(cyan2);
    addMesh(suzanne2);



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
    white.kd = 0.9f; white.ks = 0.0f; white.type=0;
    white.specularColor = QVector3D(1.0,1.0,1.0); white.shininess = 32;

    Material red;    red.color = QVector3D(0.65f,0.05f,0.05f);
    red.kd = 0.9f; red.ks = 0.0f; red.type=0;
    red.specularColor = QVector3D(1.0,1.0,1.0); red.shininess = 32;
    red.type = 1;
    Material green;  green.color = QVector3D(0.12f,0.55f,0.15f);
    green.kd = 0.9f; green.ks = 0.0f; green.type=0;
    green.specularColor = QVector3D(1.0,1.0,1.0); green.shininess = 32;

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
        m->isSquare=true;
        addMesh(m);
    };

    makeQuad(A, B, C, Dp, white);    // front
    makeQuad(F, E, Hh, G, white);    // back
    makeQuad(E, A, Dp, Hh, red);     // left
    makeQuad(B, F, G, C, green);     // right
    makeQuad(Dp, C, G, Hh, white);   // ceiling
    makeQuad(E, F, B, A, white);     // floor

    QVector<Mesh::Vertex> sVerts;
    QVector<unsigned int> sIdx;
    generateSphereMesh(1.0f, 20, 20, sVerts, sIdx);

    // Mesh* s1 = new Mesh();
    // s1->isSphere = true;
    // s1->initialize(sVerts, sIdx);
    // //s1->modelMatrix.setToIdentity();
    // s1->modelMatrix.translate(1.0f, -2.0f, 0.5f);

    // Material mirror;
    // mirror.color = QVector3D(0.9f, 0.2f, 0.2f);
    // mirror.kd = 0.8f;
    // mirror.ks = 0.2f;
    // mirror.specularColor = QVector3D(1.0,1.0,1.0);
    // mirror.shininess = 64;
    // mirror.type = 1;

    // s1->addMaterial(mirror);
    // addMesh(s1);






    Mesh* suzanne = new Mesh();
    QVector<Mesh::Vertex> meshVerts;
    QVector<unsigned int> meshIdx;
    int nbTriangle;
    QString meshFile = "../../meshes/suzanne.off";
    loadOffFile(meshFile, meshVerts, meshIdx, nbTriangle);

    suzanne->nbTriangles=nbTriangle;
    suzanne->initialize(meshVerts,meshIdx);
    suzanne->modelMatrix.translate(1.0f, -2.0f, 0.5f);
    suzanne->modelMatrix.scale(0.5);

    Material cyan;
    cyan.color=QVector3D(0,1,1);

    cyan.kd = 1.;
    cyan.ks = 0.3;
    cyan.specularColor = QVector3D(1,1,1);
    cyan.shininess = 32;
    cyan.type=1;

    suzanne->addMaterial(cyan);
    addMesh(suzanne);

    Mesh* s2 = new Mesh();
    s2->isSphere = true;
    s2->initialize(sVerts, sIdx);
    s2->modelMatrix.translate(-1.0f, -2.f, -1.0f);

    Material mate;
    mate.color = QVector3D(0.4f, 0.4f, 1.0f);
    mate.kd = 0.8f;
    mate.ks = 0.1f;
    mate.specularColor = QVector3D(1.0,1.0,1.0);
    mate.shininess = 32;
    mate.type = 2;

    s2->addMaterial(mate);
    addMesh(s2);

    Light l;
    l.position  = QVector3D(0, 2.8f, 0);
    l.color     = QVector3D(1.0,1.0,1.0);
    l.intensity = 10.0f;
    l.lightRadius = 1.1f;

    m_lights.append(l);
}

void Scene::loadOffFile(QString &fileName,
                               QVector<Mesh::Vertex> &verts,
                               QVector<unsigned int> &idx,
                                int &faceCount)
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
    // qDebug() << "Nombre triangle" << faceCount;
}

