#include "engine/render/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace engine {
Camera::Camera(float fovDeg, float aspect, float nearPlane, float farPlane)
    : m_fov(fovDeg),
      m_aspect(aspect),
      m_near(nearPlane),
      m_far(farPlane)
{
    updateView();
    updateProjection();
}

void Camera::setAspectRatio(float aspect)
{
    m_aspect = aspect;
    updateProjection();
}

void Camera::setPosition(const glm::vec3& pos)
{
    m_position = pos;
    updateView();
}

void Camera::lookAt(const glm::vec3& target)
{
    m_target = target;
    updateView();
}

const glm::mat4& Camera::getView() const
{
    return m_view;
}

const glm::mat4& Camera::getProjection() const
{
    return m_projection;
}

void Camera::updateView()
{
    m_view = glm::lookAt(m_position, m_target, m_up);
}

void Camera::updateProjection()
{
    m_projection = glm::perspective(
        glm::radians(m_fov),
        m_aspect,
        m_near,
        m_far
    );
}
} // namespace engine
