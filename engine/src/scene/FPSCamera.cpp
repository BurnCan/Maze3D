#include "engine/scene/FPSCamera.h"

#include <glm/gtc/matrix_transform.hpp>


namespace engine {
FPSCamera::FPSCamera(float fovDeg, float aspect, float nearPlane, float farPlane)
    : m_fov(fovDeg), m_aspect(aspect), m_near(nearPlane), m_far(farPlane)
{
    m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    updateVectors();
}

void FPSCamera::setAspectRatio(float aspect)
{
    m_aspect = aspect;
    m_proj = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
}



// --- Input math ---
void FPSCamera::moveForward(float amount)
{
    m_position += m_front * amount;
    updateVectors();
}

void FPSCamera::moveRight(float amount)
{
    m_position += m_right * amount;
    updateVectors();
}

void FPSCamera::moveUp(float amount)
{
    m_position.y += amount;
    updateVectors();
}

void FPSCamera::rotate(float dx, float dy)
{
    dx *= m_sensitivity;
    dy *= m_sensitivity;

    m_yaw   += dx;
    m_pitch += dy;

    m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);

    updateVectors();
}




void FPSCamera::updateVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, glm::vec3(0, 1, 0)));
    m_up    = glm::normalize(glm::cross(m_right, m_front));

    m_view = glm::lookAt(m_position, m_position + m_front, m_up);
}

const glm::mat4& FPSCamera::view() const { return m_view; }
const glm::mat4& FPSCamera::projection() const { return m_proj; }


// mesh collider implementation
void FPSCamera::setPosition(const glm::vec3& p)
{
    m_position = p;
}

glm::vec3 FPSCamera::position() const
{
    return m_position;
}


} // namespace engine

