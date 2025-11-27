#pragma once

struct GpuMaterial {
    float diffuseR, diffuseG, diffuseB, kd;
    float specularR, specularG, specularB, ks;
    float shininess; float pad1, pad2; int type;
};

struct GpuVertex {
    float px, py, pz, pad0;
    float nx, ny, nz, pad1;
};

struct GpuTriangleIndexed {
    unsigned int v0, v1, v2, matIdx;
};

struct GpuSphere {
    float cx, cy, cz, radius;
    float diffuseR, diffuseG, diffuseB, kd;
    float specularR, specularG, specularB, ks;
    float shininess; float pad1, pad2, pad3;
    int Material_type;
    int pad4, pad5, pad6;
};

struct GpuMesh {
    int Material_type, triOffset, triCount, pad0;
};

struct GPUBVHNode {
    float minX, minY, minZ;
    int rightChildOrPrim;

    float maxX, maxY, maxZ;
    int triangleCount;
};

struct GpuSquare {
    float ax, ay, az, padA;
    float bx, by, bz, padB;
    float cx, cy, cz, padC;
    float dx, dy, dz, padD;
    float diffuseR, diffuseG, diffuseB, kd;
    float specularR, specularG, specularB, ks;
    float shininess, pad1, pad2, pad3;
    int Material_type, pad4, pad5, pad6;
};


struct GpuLight {
    float px, py, pz, intensity;
    float r, g, b, pad0;
    float lightRadius, pad1, pad2, pad3;
};
