#pragma once

struct GpuSphere {
    float cx, cy, cz, radius;
    float r, g, b, pad0;
};

struct GpuLight {
    float px, py, pz, intensity;
    float r, g, b, pad0;
};
