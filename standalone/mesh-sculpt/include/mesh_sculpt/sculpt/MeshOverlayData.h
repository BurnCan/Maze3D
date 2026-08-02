#pragma once

#include <cstddef>
#include <vector>
#include <glm/glm.hpp>

namespace mesh_sculpt::sculpt {

class SculptMesh;
class MeshSelection;

struct VertexLabel
{
    std::size_t vertexIndex;
    glm::vec2 screenPosition;
};

std::vector<VertexLabel> makeMeshOverlayLabels(
    const glm::mat4& view, const glm::mat4& projection,
    const SculptMesh& mesh, const MeshSelection& selection,
    const glm::vec2& viewportMin, const glm::vec2& viewportMax);

} // namespace mesh_sculpt::sculpt
