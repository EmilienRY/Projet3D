#pragma once

struct GpuSphere {
    float cx, cy, cz, radius;
    float diffuseR, diffuseG, diffuseB, kd;
    float specularR, specularG, specularB, ks;
    float shininess; float pad1, pad2, pad3;
};


struct GpuSquare {
    float ax, ay, az, padA;
    float bx, by, bz, padB;
    float cx, cy, cz, padC;
    float dx, dy, dz, padD;
    float diffuseR, diffuseG, diffuseB, kd;
    float specularR, specularG, specularB, ks;
    float shininess, pad1, pad2, pad3;
};


struct GpuLight {
    float px, py, pz, intensity;
    float r, g, b, pad0;
};
