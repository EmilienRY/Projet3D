#pragma once
#include <QVector3D>
struct Material
{
    QVector3D color;
    QVector3D specularColor;
    float shininess;
    float kd;
    float ks;
};
