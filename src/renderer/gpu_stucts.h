#pragma once

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

struct GpuTriangle {
    float ax, ay, az, pad0;
    float bx, by, bz, pad1;
    float cx, cy, cz, pad2;
    float nx, ny, nz, pad3;
    float diffuseR, diffuseG, diffuseB, kd;
    float specularR, specularG, specularB, ks;
    float shininess; float pad4, pad5, pad6;
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
