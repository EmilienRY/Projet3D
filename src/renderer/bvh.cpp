#include "bvh.h"
#include <algorithm>
#include <iostream>

BVH::BVH() {}

void BVH::build(const std::vector<Mesh*>& meshes) {
    m_triangles.clear();
    m_nodes.clear();
    m_gpuNodes.clear();
    m_vertices.clear();
    m_materials.clear();

    for (Mesh* mesh : meshes) {
        QMatrix4x4 modelMatrix = mesh->modelMatrix;
        QMatrix3x3 normalMatrix = modelMatrix.normalMatrix();
        Material mat = mesh->material(); 

        BVHMaterial bvhMat;
        bvhMat.diffuse = mat.color;
        bvhMat.specular = mat.specularColor;
        bvhMat.shininess = mat.shininess;
        bvhMat.kd = mat.kd;
        bvhMat.ks = mat.ks;
        bvhMat.type = mat.type;
        bvhMat.texturePath = mat.texturePath;
        
        unsigned int matIdx = (unsigned int)m_materials.size();
        m_materials.push_back(bvhMat);

        const auto& verts = mesh->m_Vertices;
        const auto& indices = mesh->m_Indices;
        
        unsigned int vertexOffset = (unsigned int)m_vertices.size();

        for (const auto& v : verts) {
            BVHVertex bvhV;
            bvhV.pos = modelMatrix * v.pos;
            float nx = normalMatrix(0,0)*v.normal.x() + normalMatrix(0,1)*v.normal.y() + normalMatrix(0,2)*v.normal.z();
            float ny = normalMatrix(1,0)*v.normal.x() + normalMatrix(1,1)*v.normal.y() + normalMatrix(1,2)*v.normal.z();
            float nz = normalMatrix(2,0)*v.normal.x() + normalMatrix(2,1)*v.normal.y() + normalMatrix(2,2)*v.normal.z();
            bvhV.normal = QVector3D(nx, ny, nz).normalized();
            bvhV.uv = v.uv;

            m_vertices.push_back(bvhV);
        }

        for (int i = 0; i < indices.size(); i += 3) {
            BVHTriangle tri;
            
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i+1];
            unsigned int i2 = indices[i+2];

            tri.idx0 = vertexOffset + i0;
            tri.idx1 = vertexOffset + i1;
            tri.idx2 = vertexOffset + i2;
            tri.matIdx = matIdx;

            tri.v0 = m_vertices[tri.idx0].pos;
            tri.v1 = m_vertices[tri.idx1].pos;
            tri.v2 = m_vertices[tri.idx2].pos;
            
            tri.center = (tri.v0 + tri.v1 + tri.v2) / 3.0f;
            
            m_triangles.push_back(tri);
        }
    }
    
    if (m_triangles.empty()) return;

    m_nodes.reserve(m_triangles.size() * 2);
    m_rootIndex = build(0, m_triangles.size());

    m_gpuNodes.reserve(m_nodes.size());
    convGPU(m_rootIndex);
}

int BVH::build(int triStart, int triCount) {
    int nodeIdx = m_nodes.size();
    m_nodes.push_back(BVHNode());

    AABB boudingB;
    for (int i = 0; i < triCount; ++i) {
        const auto& tri = m_triangles[triStart + i];
        boudingB.grow(tri.v0);
        boudingB.grow(tri.v1);
        boudingB.grow(tri.v2);
    }
    m_nodes[nodeIdx].aabb = boudingB;

    if (triCount <= 4) {
        m_nodes[nodeIdx].firstTriangleIndex = triStart;
        m_nodes[nodeIdx].triangleCount = triCount;
        return nodeIdx;
    }

    int axis = boudingB.longestAxis();
    float splitPos =(axis == 0) ? boudingB.center().x() :
                    (axis == 1) ? boudingB.center().y() :
                    boudingB.center().z();

    auto separationTriangles = [axis, splitPos](const BVHTriangle& t) {
        float c = (axis == 0) ? t.center.x() :
                  (axis == 1) ? t.center.y() : t.center.z();
        return c < splitPos;
    };

    int mid = 0;
    auto it = std::partition(m_triangles.begin() + triStart,
                             m_triangles.begin() + triStart + triCount,
                             separationTriangles);

    mid = std::distance(m_triangles.begin() + triStart, it);

    if (mid == 0 || mid == triCount) {
        mid = triCount / 2;
    }

    m_nodes[nodeIdx].leftChild = build(triStart, mid);
    m_nodes[nodeIdx].rightChild = build(triStart + mid, triCount - mid);
    return nodeIdx;
}

int BVH::convGPU(int nodeIdx) {
    const BVHNode& node = m_nodes[nodeIdx];

    int gpuNodeIdx = m_gpuNodes.size();
    m_gpuNodes.push_back(GPUBVHNode());

    m_gpuNodes[gpuNodeIdx].minX = node.aabb.min.x();
    m_gpuNodes[gpuNodeIdx].minY = node.aabb.min.y();
    m_gpuNodes[gpuNodeIdx].minZ = node.aabb.min.z();

    m_gpuNodes[gpuNodeIdx].maxX = node.aabb.max.x();
    m_gpuNodes[gpuNodeIdx].maxY = node.aabb.max.y();
    m_gpuNodes[gpuNodeIdx].maxZ = node.aabb.max.z();

    if (node.isLeaf()) // Dans ce cas la rightChildOrPrim a indice du premier triangle
    {
        m_gpuNodes[gpuNodeIdx].triangleCount = node.triangleCount;
        m_gpuNodes[gpuNodeIdx].rightChildOrPrim = node.firstTriangleIndex;
    }
    else// dans ce cas la rightChildOrPrim a l'offset pour aller a neud de droite
    {
        m_gpuNodes[gpuNodeIdx].triangleCount = 0;
        convGPU(node.leftChild);

        int rightChildGPUIdx = convGPU(node.rightChild);
        m_gpuNodes[gpuNodeIdx].rightChildOrPrim = rightChildGPUIdx;
    }
    return gpuNodeIdx;
}
