#pragma once
#include <QVector3D>

struct Light
{
    QVector3D position;
    QVector3D color;
    float intensity;
};
