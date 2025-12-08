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
                        QVector<unsigned int>& idx,
                        Material mat)
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
                          QVector3D(x, y, z).normalized(),
                          mat.color });
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
        QVector3D n = QVector3D::crossProduct(b-a, c-a).normalized();
        verts.append({a, n, color});
        verts.append({b, n, color});
        verts.append({c, n, color});
        verts.append({d, n, color});
        idx.append(base+0); idx.append(base+1); idx.append(base+2);
        idx.append(base+2); idx.append(base+3); idx.append(base+0);
    };

    Material m1;
    m1.color=QVector3D(0,1,0);

    m1.kd = 1.;
    m1.ks = 0.3;
    m1.specularColor = QVector3D(1,1,1);
    m1.shininess = 32;
    m1.type=0;

    Material cyan;
    cyan.color=QVector3D(0,1,1);

    cyan.kd = 1.;
    cyan.ks = 0.3;
    cyan.specularColor = QVector3D(1,1,1);
    cyan.shininess = 32;
    cyan.type=0;

    Material cyan2;
    cyan2.color=QVector3D(1,0,0);

    cyan2.kd = 0.7;
    cyan2.ks = 0.7;
    cyan2.specularColor = QVector3D(1,1,1);
    cyan2.shininess = 32;
    cyan2.type=0;

    Material m2;
    m2.color=QVector3D(1.0, 0., 0.);;
    m2.kd = 0.5;
    m2.ks = 0.6;
    m2.specularColor = QVector3D(1.0,1.0,1.0);
    m2.shininess = 128;
    m2.type = 1;

    pushQuad(
        {-3,0.,-3},
        {-3,0.,3},
        {3,0.,3},
        {3,0.,-3},
        {0.0f, 1.0f, 0.0f}
        );

    Mesh* plane = new Mesh();
    plane->name = "plan";
    plane->addMaterial(m1);
    plane->initialize(verts, idx);
    plane->isSquare=true;
    addMesh(plane);

    QVector<Mesh::Vertex> sVerts;
    QVector<unsigned int> sIdx;

    generateSphereMesh(1.0f, 20, 20, sVerts, sIdx, m2);

    Mesh* sphere = new Mesh();
    sphere->name = "sphere";
    sphere->addMaterial(m2);
    sphere->initialize(sVerts, sIdx);
    sphere->modelMatrix.translate(0, 1, 0);
    sphere->isSphere=true;
    sphere->position.setY(1.0);
    addMesh(sphere);

    Mesh* suzanne = new Mesh();
    suzanne->name = "suzanne";
    QVector<Mesh::Vertex> meshVerts;
    QVector<unsigned int> meshIdx;
    int nbTriangle;
    QString meshFile = "../../meshes/suzanne.obj";
    loadObjFile(meshFile, meshVerts, meshIdx, nbTriangle,cyan);

    suzanne->nbTriangles=nbTriangle;
    suzanne->initialize(meshVerts,meshIdx);

    suzanne->modelMatrix.translate(-1, 1, 2);
    suzanne->position.setX(-1.0f);
    suzanne->position.setY(1.0f);
    suzanne->position.setZ(2.0f);

    suzanne->modelMatrix.scale(0.5);
    suzanne->scale=0.5;

    suzanne->addMaterial(cyan);
    addMesh(suzanne);

    Mesh* suzanne2 = new Mesh();
    suzanne2->name = "suzanne1";
    QVector<Mesh::Vertex> meshVerts2;
    QVector<unsigned int> meshIdx2;
    int nbTriangle2;
    QString meshFile2 = "../../meshes/suzanne.obj";
    loadObjFile(meshFile2, meshVerts2, meshIdx2, nbTriangle2, cyan2);

    suzanne2->nbTriangles=nbTriangle2;
    suzanne2->initialize(meshVerts2,meshIdx2);

    suzanne2->modelMatrix.translate(1, 1, 2);
    suzanne2->position.setX(1.0f);
    suzanne2->position.setY(1.0f);
    suzanne2->position.setZ(2.0f);

    suzanne2->modelMatrix.scale(0.5);
    suzanne2->scale=0.5;

    suzanne2->addMaterial(cyan2);
    addMesh(suzanne2);


    Light l;
    l.position = QVector3D(2.0f, 4.0f, 2.0f);
    l.color    = QVector3D(1.0f, 1.f, 1.f);
    l.intensity= 25.2f;
    l.lightRadius = 1.1f;
    m_lights.append(l);

    qDebug() << "nb mesh dans la scène" << m_meshes.size();
    for (int i = 0; i < m_meshes.size(); ++i) {
        qDebug() << "nb vertice dans le mesh" << m_meshes[i]->m_Vertices.size();
        for (int j = 0; j < m_meshes[i]->m_Vertices.size(); ++j) {
            m_meshes[i]->m_Vertices[j].color = m_meshes[i]->material().color;
            qDebug() << "Vertex Color" << m_meshes[i]->m_Vertices[j].color;
        }
    }
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
    white.texturePath = "../../textures/carrelage.jpg";


    Material red;    red.color = QVector3D(0.65f,0.05f,0.05f);
    red.kd = 0.9f; red.ks = 0.0f; red.type=0;
    red.specularColor = QVector3D(1.0,1.0,1.0); red.shininess = 32;
    red.type = 0;

    Material verre;
    verre.color = QVector3D(1.f, 1.f, 1.f);
    verre.kd = 1.0f;
    verre.ks = 0.0f;
    verre.specularColor = QVector3D(0,0,0);
    verre.shininess = 1;
    verre.type = 2;


    Material mirror;
    mirror.color = QVector3D(1.f, 1.f, 1.f);
    mirror.kd = 0.0f;
    mirror.ks = 1.0f;
    mirror.specularColor = QVector3D(1.0f, 1.0f, 1.0f);
    mirror.shininess = 128;
    mirror.type = 1;


    Material green;  green.color = QVector3D(0.12f,0.55f,0.15f);
    green.kd = 0.9f; green.ks = 0.0f; green.type=0;
    green.specularColor = QVector3D(1.0,1.0,1.0); green.shininess = 32;

    Material matteBlue;
    matteBlue.color = QVector3D(0.2f, 0.2f, 0.9f);
    matteBlue.kd = 1.0f;
    matteBlue.ks = 0.0f;
    matteBlue.specularColor = QVector3D(0,0,0);
    matteBlue.shininess = 1;
    matteBlue.type = 0;
    matteBlue.texturePath = "../../textures/sanic.png";

    Material matteGreen;
    matteGreen.color = QVector3D(0.2f, 0.2f, 0.9f);
    matteGreen.kd = 1.0f;
    matteGreen.ks = 0.0f;
    matteGreen.specularColor = QVector3D(0,0,0);
    matteGreen.shininess = 1;
    matteGreen.type = 0;

    matteGreen.texturePath = "../../textures/sanic.png";

    Material matteRed;
    matteRed.color = QVector3D(1.0f, 0.0f, 0.0f);
    matteRed.kd = 1.0f;
    matteRed.ks = 0.0f;
    matteRed.specularColor = QVector3D(0,0,0);
    matteRed.shininess = 1;
    matteRed.type = 0;

    auto makeQuad = [&](QVector3D a, QVector3D b, QVector3D c, QVector3D d, Material mat, QString name)
    {
        QVector<Mesh::Vertex> verts;
        QVector<unsigned int> idx;

        unsigned int base = 0;

        QVector3D n = QVector3D::crossProduct(c-d, b-d).normalized();

        verts.append({ d, n, mat.color });
        verts.append({ c, n, mat.color });
        verts.append({ b, n, mat.color });
        verts.append({ a, n, mat.color });

        idx.append(base+0); idx.append(base+1); idx.append(base+2);
        idx.append(base+2); idx.append(base+3); idx.append(base+0);

        Mesh* m = new Mesh();
        m->initialize(verts, idx);
        m->modelMatrix.setToIdentity();
        m->addMaterial(mat);
        m->isSquare=true;
        m->name = name;
        addMesh(m);
    };

    makeQuad(A, B, C, Dp, mirror, "front");    // front
    makeQuad(F, E, Hh, G, mirror, "back");    // back
    makeQuad(E, A, Dp, Hh, red, "left");     // left
    makeQuad(B, F, G, C, green, "right");     // right
    makeQuad(Dp, C, G, Hh, white, "ceiling");   // ceiling
    makeQuad(E, F, B, A, white, "floor");     // floor

    QVector<Mesh::Vertex> sVerts;
    QVector<unsigned int> sIdx;

    Mesh* suzanne = new Mesh();
    QVector<Mesh::Vertex> meshVerts;
    QVector<unsigned int> meshIdx;
    int nbTriangle;
    QString meshFile = "../../meshes/chat.obj";
    loadObjFile(meshFile, meshVerts, meshIdx, nbTriangle,matteGreen);
    suzanne->name = "chat";

    suzanne->nbTriangles=nbTriangle;
    suzanne->initialize(meshVerts,meshIdx);
    suzanne->modelMatrix.translate(1.0f, -2.0f, 0.5f);
    suzanne->position = QVector3D (1.0f, -2.0f, 0.5f);

    suzanne->modelMatrix.scale(0.5);
    suzanne->scale=0.5f;

    // Material gold;
    // gold.color = QVector3D(1.0f, 0.78f, 0.34f);
    // gold.kd = 0.0f;
    // gold.ks = 1.0f;
    // gold.specularColor = gold.color;
    // gold.shininess = 128;
    // gold.type = 1;

    suzanne->addMaterial(matteGreen);
    addMesh(suzanne);

    generateSphereMesh(1.0f, 20, 20, sVerts, sIdx,matteBlue);

    Mesh* s2 = new Mesh();
    s2->name = "sphere sanic";
    s2->isSphere = true;
    s2->initialize(sVerts, sIdx);
    s2->modelMatrix.translate(-2.f, 2.0f, -2.0f);
    s2->position = QVector3D(-2.0f, 2.0f, -2.0f);

    s2->addMaterial(matteBlue);
    addMesh(s2);

    generateSphereMesh(1.0f, 20, 20, sVerts, sIdx,verre);

    Mesh* s3 = new Mesh();
    s3->name = "sphere verre";
    s3->isSphere = true;
    s3->initialize(sVerts, sIdx);
    s3->modelMatrix.translate(-1.0f, -2.0f, 1.0f);
    s3->position = QVector3D(-1.0f, -2.0f, 1.0f);
    s3->addMaterial(verre);
    addMesh(s3);

    Mesh* squirrel = new Mesh();
    squirrel->name = "suzanne";
    QVector<Mesh::Vertex> meshVertsSquirrel;
    QVector<unsigned int> meshIdxSquirrel;
    int nbTrianglesSquirrel;
    QString meshFileSquirrel = "../../meshes/suzanne.obj";
    loadObjFile(meshFileSquirrel, meshVertsSquirrel, meshIdxSquirrel, nbTrianglesSquirrel, matteRed);

    squirrel->nbTriangles=nbTrianglesSquirrel;
    squirrel->initialize(meshVertsSquirrel,meshIdxSquirrel);

    squirrel->modelMatrix.translate(1.0f, -2.0f, -2.f);
    squirrel->position = QVector3D(1.0f, -2.0f, -2.0f);

    squirrel->modelMatrix.scale(1.2f);
    squirrel->scale = 1.2f;

    squirrel->addMaterial(matteRed);
    addMesh(squirrel);

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

    for (auto &p : positions) {
        Mesh::Vertex v;
        v.pos = p;
        v.normal = QVector3D(0, 0, 0);
        v.color = defaultColor;
        verts.append(v);
    }

    for (int i = 0; i < faceCount; ++i) {
        int n, a, b, c;
        in >> n >> a >> b >> c;

        if (n != 3) {
            qWarning() << "Non triangular face encountered. Only triangles are supported!";
        }

        idx.append(static_cast<unsigned int>(a));
        idx.append(static_cast<unsigned int>(b));
        idx.append(static_cast<unsigned int>(c));

        QVector3D vA = verts[a].pos;
        QVector3D vB = verts[b].pos;
        QVector3D vC = verts[c].pos;
        QVector3D faceNormal = QVector3D::crossProduct(vB - vA, vC - vA);

        verts[a].normal += faceNormal;
        verts[b].normal += faceNormal;
        verts[c].normal += faceNormal;
    }

    for (int i = 0; i < verts.size(); ++i) {
        verts[i].normal.normalize();
    }
}



void Scene::loadObjFile(const QString &fileName,
                        QVector<Mesh::Vertex> &verts,
                        QVector<unsigned int> &idx,
                        int &faceCount,
                        Material mat)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open OBJ file:" << fileName;
        return;
    }

    QTextStream in(&file);

    QVector<QVector3D> positions;
    QVector<QVector3D> normals;
    QVector<QVector2D> uvs;

    verts.clear();
    idx.clear();
    faceCount = 0;

    QVector3D color;

    if (mat.color != QVector3D(0.0f, 0.0f, 0.0f)){
        color = mat.color;
    }
    else {
        color = QVector3D (1.0f, 1.0f, 1.0f);
    }


    QVector3D defaultNormal(0, 0, 0);
    QVector2D defaultUV(0, 0);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#"))
            continue;

        QStringList tokens = line.split(' ', Qt::SkipEmptyParts);
        if (tokens.isEmpty())
            continue;

        if (tokens[0] == "v") {
            // Vertex position
            if (tokens.size() < 4) continue;
            float x = tokens[1].toFloat();
            float y = tokens[2].toFloat();
            float z = tokens[3].toFloat();
            positions.append(QVector3D(x, y, z));
        }
        else if (tokens[0] == "vn") {
            // Vertex normal
            if (tokens.size() < 4) continue;
            float x = tokens[1].toFloat();
            float y = tokens[2].toFloat();
            float z = tokens[3].toFloat();
            normals.append(QVector3D(x, y, z));
        }
        else if (tokens[0] == "vt") {
            // Vertex UV
            if (tokens.size() < 3) continue;
            float u = tokens[1].toFloat();
            float v = tokens[2].toFloat();
            uvs.append(QVector2D(u, v));
        }
        else if (tokens[0] == "f") {
            // Face
            if (tokens.size() < 4) continue; // ignore non-triangles for now
            faceCount++;

            for (int i = 1; i <= 3; ++i) {
                QStringList parts = tokens[i].split('/');
                int vi = parts[0].toInt() - 1; // vertex index
                int ti = parts.size() > 1 && !parts[1].isEmpty() ? parts[1].toInt() - 1 : -1; // uv index
                int ni = parts.size() > 2 ? parts[2].toInt() - 1 : -1; // normal index

                Mesh::Vertex v;
                v.pos = positions[vi];
                v.color = color;
                v.normal = (ni >= 0 && ni < normals.size()) ? normals[ni] : defaultNormal;
                v.uv = (ti >= 0 && ti < uvs.size()) ? uvs[ti] : defaultUV;

                verts.append(v);
                idx.append(static_cast<unsigned int>(verts.size() - 1));
            }
        }
    }

    if (normals.isEmpty()) {
        for (int i = 0; i < idx.size(); i += 3) {
            Mesh::Vertex &a = verts[idx[i]];
            Mesh::Vertex &b = verts[idx[i + 1]];
            Mesh::Vertex &c = verts[idx[i + 2]];

            QVector3D faceNormal = QVector3D::crossProduct(b.pos - a.pos, c.pos - a.pos);
            a.normal += faceNormal;
            b.normal += faceNormal;
            c.normal += faceNormal;
        }

        for (auto &v : verts) {
            v.normal.normalize();
        }
    }
}


