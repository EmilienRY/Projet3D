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

void Mesh::initialize(const QVector<Vertex> &vertices, const QVector<unsigned int> &indices)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(vertices.constData(), vertices.size() * sizeof(Vertex));

    m_ibo.create();
    m_ibo.bind();
    m_ibo.allocate(indices.constData(), indices.size() * sizeof(unsigned int));
    m_indexCount = indices.size();

    // vertex attributes will be enabled/linked by the shader program before rendering
    m_vao.release();
    m_vbo.release();
    m_ibo.release();
}

void Mesh::render()
{
    m_vao.bind();
    m_vbo.bind();
    m_ibo.bind();

    // assume shader attributes are at location 0 (pos) and 1 (color)
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, pos)));
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));

    f->glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);

    f->glDisableVertexAttribArray(0);
    f->glDisableVertexAttribArray(1);

    m_ibo.release();
    m_vbo.release();
    m_vao.release();
}
