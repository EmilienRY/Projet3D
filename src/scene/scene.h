#pragma once

#include <QVector>
#include "light.h"
#include "mesh.h"

class Scene
{
public:
    Scene();
    ~Scene();

    void addMesh(Mesh* m);
    void clear();
    const QVector<Mesh*>& meshes() const { return m_meshes; }
    const QVector<Light>& lights() const { return m_lights; }

    void buildPlaneSphere();
    void buildCornellBox();
    void loadOffFile(QString &fileName, QVector<Mesh::Vertex> &verts, QVector<unsigned int> &idx, int &faceCount);

    void loadObjFile(const QString &fileName,QVector<Mesh::Vertex> &verts,QVector<unsigned int> &idx,int &faceCount);
private:
    QVector<Mesh*> m_meshes;
    QVector<Light> m_lights;
};
