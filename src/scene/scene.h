#pragma once

#include <QVector>
#include "light.h"

class Mesh;

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

private:
    QVector<Mesh*> m_meshes;
    QVector<Light> m_lights;
};
