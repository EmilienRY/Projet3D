#include "camera.h"
#include <QtMath>

Camera::Camera()
    : m_pos(0.0f, 0.0f, 3.0f),
    m_yaw(-90.0f),
    m_pitch(0.0f),
    m_front(0.0f, 0.0f, -1.0f),
    m_up(0.0f, 1.0f, 0.0f),
    m_moveSpeed(5.0f),
    m_mouseSensitivity(0.1f)
{
    updateVectors();
    setPerspective(60.0f, 4.0f/3.0f, 0.1f, 100.0f);
}

void Camera::setPosition(const QVector3D &pos) { m_pos = pos; }

void Camera::setYawPitch(float yaw, float pitch)
{
    m_yaw = yaw;
    m_pitch = qBound(-89.0f, pitch, 89.0f);
    updateVectors();
}

void Camera::processMouseMovement(float deltaX, float deltaY)
{
    m_yaw += deltaX * m_mouseSensitivity;
    m_pitch += deltaY * m_mouseSensitivity;
    m_pitch = qBound(-89.0f, m_pitch, 89.0f);
    updateVectors();
}

void Camera::processKeyboard(const QVector3D &direction, float deltaTime)
{
    QVector3D dir = direction;
    if (!qFuzzyIsNull(dir.lengthSquared()))
        dir.normalize();
    m_pos += dir * m_moveSpeed * deltaTime;
}

QMatrix4x4 Camera::viewMatrix() const
{
    QMatrix4x4 view;
    view.lookAt(m_pos, m_pos + m_front, m_up);
    return view;
}

void Camera::setPerspective(float fovDeg, float aspect, float nearp, float farp)
{
    m_proj.setToIdentity();
    m_proj.perspective(fovDeg, aspect, nearp, farp);
}

void Camera::updateVectors()
{
    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);

    QVector3D front;
    front.setX(qCos(yawRad) * qCos(pitchRad));
    front.setY(qSin(pitchRad));
    front.setZ(qSin(yawRad) * qCos(pitchRad));
    m_front = front.normalized();

    m_right = QVector3D::crossProduct(m_front, QVector3D(0.0f, 1.0f, 0.0f)).normalized();
    m_up = QVector3D::crossProduct(m_right, m_front).normalized();
}
