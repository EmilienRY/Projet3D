#include "scene.h"
#include "mesh.h"

Scene::Scene()
{
}

Scene::~Scene()
{
    // détruit les Mesh que la scène possède
    for (Mesh* m : m_meshes) {
        delete m;
    }
    m_meshes.clear();
}

void Scene::addMesh(Mesh* m)
{
    if (m) m_meshes.append(m);
}
