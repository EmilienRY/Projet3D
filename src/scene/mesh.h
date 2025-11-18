#pragma once
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include <QMatrix4x4>
#include "material.h"

class Mesh
{
public:
    struct Vertex {
        QVector3D pos;
        QVector3D color;
    };

    Mesh();
    ~Mesh();

    void initialize(const QVector<Vertex>& vertices, const QVector<unsigned int>& indices);
    void render();
    Material material(){return m_material;}
    QMatrix4x4 modelMatrix;

    void addMaterial(const Material& m);
    bool isSphere=false;
private:
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ibo;
    QOpenGLVertexArrayObject m_vao;
    int m_indexCount;
    Material m_material;
};
