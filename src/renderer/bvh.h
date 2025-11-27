#pragma once

#include <vector>
#include <QVector3D>
#include <limits>
#include "../scene/mesh.h"
#include "gpu_stucts.h"


struct BVHVertex {
    QVector3D pos;
    QVector3D normal;
};

struct BVHMaterial {
    QVector3D diffuse;
    QVector3D specular;
    float shininess;
    float kd, ks;
    int type;
};

struct BVHTriangle {
    QVector3D v0, v1, v2;
    QVector3D center; 
    
    unsigned int idx0, idx1, idx2;
    unsigned int matIdx;
};

struct AABB {
    QVector3D min;
    QVector3D max;

    AABB() {
        min = QVector3D(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        max = QVector3D(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    }

    void grow(const QVector3D& p) {
        min.setX(std::min(min.x(), p.x()));
        min.setY(std::min(min.y(), p.y()));
        min.setZ(std::min(min.z(), p.z()));
        max.setX(std::max(max.x(), p.x()));
        max.setY(std::max(max.y(), p.y()));
        max.setZ(std::max(max.z(), p.z()));
    }

    void grow(const AABB& other) {
        grow(other.min);
        grow(other.max);
    }

    QVector3D center() const { return (min + max) * 0.5f; }

    int longestAxis() const {
        QVector3D e = max - min;
        if (e.x() > e.y() && e.x() > e.z()) return 0;
        if (e.y() > e.z()) return 1;
        return 2;
    }
};

struct BVHNode {
    AABB aabb;
    int leftChild = -1;
    int rightChild = -1;
    int firstTriangleIndex = -1;
    int triangleCount = 0;

    bool isLeaf() const { return triangleCount > 0; }
};

class BVH {
public:
    BVH();

    void build(const std::vector<Mesh*>& meshes);

    const std::vector<GPUBVHNode>& getGPUNodes() const { return m_gpuNodes; }

    const std::vector<BVHTriangle>& getTriangles() const { return m_triangles; }

    const std::vector<BVHVertex>& getVertices() const { return m_vertices; }
    const std::vector<BVHMaterial>& getMaterials() const { return m_materials; }

private:

    int build(int triStart, int triCount);

    int convGPU(int nodeIdx);

    std::vector<BVHTriangle> m_triangles;
    std::vector<BVHNode> m_nodes;
    std::vector<GPUBVHNode> m_gpuNodes;
    std::vector<BVHVertex> m_vertices;
    std::vector<BVHMaterial> m_materials;
    int m_rootIndex = 0;
};
