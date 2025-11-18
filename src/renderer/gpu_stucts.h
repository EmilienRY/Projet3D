#pragma once

struct GpuSphere {
    float cx, cy, cz, radius;
    float r, g, b, pad0;
};

struct GpuSquare {
    float ax, ay, az, pada;
    float bx, by, bz, padb;
    float cx, cy, cz, padc;
    float dx, dy, dz, padd;
    float r, g, b, pad0;
};

struct GpuLight {
    float px, py, pz, intensity;
    float r, g, b, pad0;
};
