#pragma once
#include <QVector3D>
#include <QString>

struct Material
{
    QVector3D color;
    QString texturePath="";
    QString normalMapPath="";
    QVector3D specularColor;
    float shininess;
    float kd;
    float ks;
    int type;

    QVector3D emissionColor = QVector3D(0,0,0);
    float emissionStrength = 0.0f;
};
