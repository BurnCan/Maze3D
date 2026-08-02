#include "veometri/sculpt/VeometriRenderData.h"

#include "veometri/sculpt/SculptMesh.h"

#include <limits>

namespace veometri::sculpt {

std::optional<std::array<glm::vec3, 3>> selectedTrianglePositions(
    const SculptMesh& mesh, std::size_t triangleIndex) noexcept
{
    if (triangleIndex > (std::numeric_limits<std::size_t>::max() - 2) / 3)
        return std::nullopt;

    const std::size_t base = triangleIndex * 3;
    const auto& indices = mesh.indices();
    const auto& vertices = mesh.vertices();
    if (base + 2 >= indices.size() ||
        indices[base] >= vertices.size() ||
        indices[base + 1] >= vertices.size() ||
        indices[base + 2] >= vertices.size())
        return std::nullopt;

    return std::array<glm::vec3, 3>{
        vertices[indices[base]], vertices[indices[base + 1]], vertices[indices[base + 2]]};
}

std::optional<glm::vec3> selectedVertexPosition(
    const SculptMesh& mesh, std::size_t vertexIndex) noexcept
{
    if (vertexIndex >= mesh.vertices().size())
        return std::nullopt;
    return mesh.vertices()[vertexIndex];
}

} // namespace veometri::sculpt
