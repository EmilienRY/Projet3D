#pragma once
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include "material.h"

class Mesh
{
public:
    struct Vertex {
        QVector3D pos;
        QVector3D normal;
        QVector3D color;
        QVector2D uv;
    };

    Mesh();
    ~Mesh();

    QString name;

    void initialize(const QVector<Vertex>& vertices, const QVector<unsigned int>& indices);
    void render();
    Material material(){return m_material;}
    QMatrix4x4 modelMatrix;
    QVector3D position = {0, 0, 0};
    QVector3D rotation = {0, 0, 0};
    float scale = 1.0f;

    void addMaterial(const Material& m);
    bool isSphere=false;
    bool isSquare=false;

    QVector<Vertex> m_Vertices;
    QVector<unsigned int> m_Indices;
    int nbTriangles;
    void updateModelMatrix();

private:
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_ibo;
    QOpenGLVertexArrayObject m_vao;
    int m_indexCount;
    Material m_material;
};
