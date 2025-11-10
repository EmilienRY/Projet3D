#pragma once

#include <QVector>

class Mesh;

class Scene
{
public:
    Scene();
    ~Scene();

    // ownership : Scene prend la possession du Mesh* (sera delete dans le destruteur)
    void addMesh(Mesh* m);

    // accès en lecture seule à la liste de meshes
    const QVector<Mesh*>& meshes() const { return m_meshes; }

private:
    QVector<Mesh*> m_meshes;
};
