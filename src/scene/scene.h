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
    void addLight(Light l);
    void clear();
    const QVector<Mesh*>& meshes() const { return m_meshes; }
    const QVector<Light>& lights() const { return m_lights; }
    QVector<Light>& lights() { return m_lights; }

    void generateSphereMesh(float radius, int stacks, int slices, QVector<Mesh::Vertex>& verts, QVector<unsigned int>& idx, Material mat);
    void GenerateQuad(QVector3D a, QVector3D b, QVector3D c, QVector3D d, QVector3D color, QVector<Mesh::Vertex>& verts, QVector<unsigned int>& idx);

    void buildPlaneSphere();
    void buildCornellBox();
    void buildEmptyScene();
    void loadObjFile(const QString &fileName,QVector<Mesh::Vertex> &verts,QVector<unsigned int> &idx,int &faceCount, Material mat);

private:
    QVector<Mesh*> m_meshes;
    QVector<Light> m_lights;
};
