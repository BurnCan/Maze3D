#include "veometri/sculpt/MeshPicker.h"

#include "veometri/render/Camera.h"
#include "veometri/sculpt/SculptMesh.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace veometri::sculpt {
namespace {

std::optional<glm::vec3> normalized(const glm::vec3& value) noexcept
{
    const float lengthSquared = glm::dot(value, value);
    if (!(lengthSquared > 0.0f) || !std::isfinite(lengthSquared))
        return std::nullopt;
    return value / std::sqrt(lengthSquared);
}

bool rayIntersectsTriangle(const Ray& ray, const glm::vec3& v0,
                           const glm::vec3& v1, const glm::vec3& v2,
                           float epsilon, float minimumDistance, float& t) noexcept
{
    const glm::vec3 edge1 = v1 - v0;
    const glm::vec3 edge2 = v2 - v0;
    const glm::vec3 h = glm::cross(ray.direction, edge2);
    const float determinant = glm::dot(edge1, h);
    // The original implementation accepted both windings, so picking is double-sided.
    if (std::abs(determinant) < epsilon)
        return false;

    const float inverse = 1.0f / determinant;
    const glm::vec3 s = ray.origin - v0;
    const float u = inverse * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;

    const glm::vec3 q = glm::cross(s, edge1);
    const float v = inverse * glm::dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = inverse * glm::dot(edge2, q);
    return t > std::max(epsilon, minimumDistance);
}

} // namespace

std::optional<Ray> makeCameraForwardRay(const render::Camera& camera) noexcept
{
    const auto direction = normalized(camera.forward());
    if (!direction)
        return std::nullopt;
    return Ray{camera.position(), *direction};
}

std::optional<Ray> makeCameraRay(const render::Camera& camera,
                                 const glm::vec2& cursorPosition,
                                 const Viewport& viewport) noexcept
{
    const glm::vec2 size = viewport.maximum - viewport.minimum;
    if (!(size.x > 0.0f) || !(size.y > 0.0f))
        return std::nullopt;

    const glm::vec2 relative = (cursorPosition - viewport.minimum) / size;
    const glm::vec2 ndc(relative.x * 2.0f - 1.0f, 1.0f - relative.y * 2.0f);
    const glm::mat4 inverseViewProjection = glm::inverse(camera.projection() * camera.view());
    glm::vec4 nearPoint = inverseViewProjection * glm::vec4(ndc, -1.0f, 1.0f);
    glm::vec4 farPoint = inverseViewProjection * glm::vec4(ndc, 1.0f, 1.0f);
    if (std::abs(nearPoint.w) <= 1.0e-7f || std::abs(farPoint.w) <= 1.0e-7f)
        return std::nullopt;
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;
    const auto direction = normalized(glm::vec3(farPoint - nearPoint));
    if (!direction)
        return std::nullopt;
    return Ray{camera.position(), *direction};
}

std::optional<std::size_t> MeshPicker::pickVertex(const SculptMesh& mesh, const Ray& ray) const noexcept
{
    const auto direction = normalized(ray.direction);
    if (!direction)
        return std::nullopt;

    float bestDistance = m_settings.vertexPickRadius;
    std::optional<std::size_t> bestIndex;
    const auto& vertices = mesh.vertices();
    for (std::size_t index = 0; index < vertices.size(); ++index)
    {
        const glm::vec3 toVertex = vertices[index] - ray.origin;
        const float t = glm::dot(toVertex, *direction);
        if (t < m_settings.minimumRayDistance)
            continue;
        const float distance = glm::length(vertices[index] - (ray.origin + *direction * t));
        // Strict comparison preserves the 0.1 world-unit threshold and makes ties
        // deterministic in favor of the first vertex.
        if (distance < bestDistance)
        {
            bestDistance = distance;
            bestIndex = index;
        }
    }
    return bestIndex;
}

std::optional<std::size_t> MeshPicker::pickTriangle(const SculptMesh& mesh, const Ray& ray) const noexcept
{
    const auto direction = normalized(ray.direction);
    if (!direction)
        return std::nullopt;
    const Ray normalizedRay{ray.origin, *direction};
    const auto& vertices = mesh.vertices();
    const auto& indices = mesh.indices();
    float closest = std::numeric_limits<float>::max();
    std::optional<std::size_t> result;
    for (std::size_t offset = 0; offset + 2 < indices.size(); offset += 3)
    {
        const auto i0 = indices[offset];
        const auto i1 = indices[offset + 1];
        const auto i2 = indices[offset + 2];
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size())
            continue;
        float distance = 0.0f;
        if (rayIntersectsTriangle(normalizedRay, vertices[i0], vertices[i1], vertices[i2],
                                  m_settings.epsilon, m_settings.minimumRayDistance, distance) &&
            distance < closest)
        {
            closest = distance;
            result = offset / 3;
        }
    }
    return result;
}

} // namespace veometri::sculpt
