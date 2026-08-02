#include "veometri/sculpt/VertexDragManipulator.h"

#include <cmath>

#include <glm/geometric.hpp>

namespace veometri::sculpt {
namespace {

bool finite(const glm::vec3& value) noexcept
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

std::optional<float> intersectRayPlane(const Ray& ray, const glm::vec3& normal,
                                       float distance, float epsilon) noexcept
{
    if (!finite(ray.origin) || !finite(ray.direction))
        return std::nullopt;

    const float denominator = glm::dot(normal, ray.direction);
    if (!std::isfinite(denominator) || std::abs(denominator) < epsilon)
        return std::nullopt;

    const float t = (distance - glm::dot(normal, ray.origin)) / denominator;
    if (!std::isfinite(t) || t < 0.0f)
        return std::nullopt;
    return t;
}

} // namespace

VertexDragManipulator::VertexDragManipulator(Settings settings) noexcept
    : m_settings(settings)
{
}

bool VertexDragManipulator::begin(const glm::vec3& vertexPosition,
                                  const glm::vec3& dragPlaneNormal,
                                  const Ray& initialRay) noexcept
{
    end();
    if (!finite(vertexPosition) || !finite(dragPlaneNormal) ||
        !std::isfinite(m_settings.epsilon) || !(m_settings.epsilon > 0.0f))
        return false;

    const float lengthSquared = glm::dot(dragPlaneNormal, dragPlaneNormal);
    if (!std::isfinite(lengthSquared) ||
        !(lengthSquared > m_settings.epsilon * m_settings.epsilon))
        return false;

    m_planeNormal = dragPlaneNormal / std::sqrt(lengthSquared);
    m_planeDistance = glm::dot(m_planeNormal, vertexPosition);
    if (!std::isfinite(m_planeDistance) ||
        !intersectRayPlane(initialRay, m_planeNormal, m_planeDistance, m_settings.epsilon))
    {
        end();
        return false;
    }

    m_active = true;
    return true;
}

std::optional<glm::vec3> VertexDragManipulator::update(const Ray& currentRay) const noexcept
{
    if (!m_active)
        return std::nullopt;
    const auto distance = intersectRayPlane(currentRay, m_planeNormal,
                                            m_planeDistance, m_settings.epsilon);
    if (!distance)
        return std::nullopt;
    const glm::vec3 position = currentRay.origin + currentRay.direction * *distance;
    return finite(position) ? std::optional<glm::vec3>(position) : std::nullopt;
}

void VertexDragManipulator::end() noexcept
{
    m_active = false;
    m_planeNormal = glm::vec3(0.0f);
    m_planeDistance = 0.0f;
}

bool VertexDragManipulator::isActive() const noexcept
{
    return m_active;
}

} // namespace veometri::sculpt
