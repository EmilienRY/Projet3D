#pragma once
#include <QVector3D>
#include <QMatrix4x4>

class Camera
{
public:
    Camera();

    void setPosition(const QVector3D& pos);
    const QVector3D& position() const { return m_pos; }

    void setYawPitch(float yaw, float pitch);
    void processMouseMovement(float deltaX, float deltaY);
    void processKeyboard(const QVector3D& direction, float deltaTime);
    const QVector3D& front() const { return m_front; }
    const QVector3D& right() const { return m_right; }
    const QVector3D& up() const { return m_up; }

    QMatrix4x4 viewMatrix() const;
    void setPerspective(float fovDeg, float aspect, float nearp, float farp);

private:
    void updateVectors();

    QVector3D m_pos;
    float m_yaw;   // degrees
    float m_pitch; // degrees
    QVector3D m_front;
    QVector3D m_right;
    QVector3D m_up;
    QMatrix4x4 m_proj;
    float m_moveSpeed;
    float m_mouseSensitivity;
};
