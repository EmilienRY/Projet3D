#pragma once
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include <QMatrix4x4>

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

    QMatrix4x4 modelMatrix;

private:
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ibo;
    QOpenGLVertexArrayObject m_vao;
    int m_indexCount;
};
