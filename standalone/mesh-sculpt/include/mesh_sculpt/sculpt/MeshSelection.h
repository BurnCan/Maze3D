#pragma once

#include <cstddef>
#include <optional>

namespace mesh_sculpt::sculpt {

class SculptMesh;

// Vertex and triangle selections are independent and may coexist. This matches
// the tool's existing behavior: successfully picking a vertex does not discard
// an already-selected triangle.
class MeshSelection
{
public:
    using Index = std::size_t;

    std::optional<Index> selectedVertex() const noexcept { return m_selectedVertex; }
    std::optional<Index> selectedTriangle() const noexcept { return m_selectedTriangle; }

    void selectVertex(Index index) noexcept { m_selectedVertex = index; }
    void selectTriangle(Index index) noexcept { m_selectedTriangle = index; }
    void clearVertex() noexcept { m_selectedVertex.reset(); }
    void clearTriangle() noexcept { m_selectedTriangle.reset(); }
    void clear() noexcept;
    void validateAgainst(const SculptMesh& mesh) noexcept;

private:
    std::optional<Index> m_selectedVertex;
    std::optional<Index> m_selectedTriangle;
};

} // namespace mesh_sculpt::sculpt
