#pragma once
#include <QVector3D>
#include <QString>

struct Material
{
    QVector3D color;
    QString texturePath="";
    QVector3D specularColor;
    float shininess;
    float kd;
    float ks;
    int type;
};
