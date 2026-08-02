#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include <glm/vec3.hpp>

namespace mesh_sculpt::sculpt {

class SculptMesh;

std::optional<std::array<glm::vec3, 3>> selectedTrianglePositions(
    const SculptMesh& mesh, std::size_t triangleIndex) noexcept;
std::optional<glm::vec3> selectedVertexPosition(
    const SculptMesh& mesh, std::size_t vertexIndex) noexcept;

} // namespace mesh_sculpt::sculpt
