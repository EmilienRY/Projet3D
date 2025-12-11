#include "mesh.h"
#include <QOpenGLFunctions>

Mesh::Mesh()
    : m_vbo(QOpenGLBuffer::VertexBuffer),
    m_ibo(QOpenGLBuffer::IndexBuffer),
    m_indexCount(0)
{
    modelMatrix.setToIdentity();
}

Mesh::~Mesh()
{
    m_vao.destroy();
    m_vbo.destroy();
    m_ibo.destroy();
}

void Mesh::addMaterial(const Material& m)
{
    m_material=m;
}

void Mesh::initialize(const QVector<Vertex> &vertices, const QVector<unsigned int> &indices)
{
    m_Vertices = vertices;
    m_Indices = indices;
    
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (!ctx) return;
    
    QOpenGLFunctions *f = ctx->functions();

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));

    m_ibo.create();
    m_ibo.bind();
    m_ibo.allocate(indices.constData(), indices.size() * sizeof(unsigned int));
    m_indexCount = indices.size();

    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glEnableVertexAttribArray(2);
    
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, pos)));
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, normal)));
    f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));

    m_vao.release();
    m_vbo.release();
    m_ibo.release();
}

void Mesh::render()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    
    m_vao.bind();
    f->glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    m_vao.release();
}

void Mesh::updateModelMatrix()
{
    modelMatrix.setToIdentity();
    modelMatrix.translate(position);

    modelMatrix.rotate(rotation.x(), 1, 0, 0);
    modelMatrix.rotate(rotation.y(), 0, 1, 0);
    modelMatrix.rotate(rotation.z(), 0, 0, 1);

    modelMatrix.scale(scale);
}
