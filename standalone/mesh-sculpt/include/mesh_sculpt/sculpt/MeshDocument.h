#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <glm/vec3.hpp>

#include "mesh_sculpt/sculpt/MeshSelection.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"

namespace mesh_sculpt::sculpt {

// Owns all authoritative editable geometry and selection state. Dirty state is
// derived from an exact snapshot of the geometry at the last markSaved().
class MeshDocument
{
public:
    struct MutationResult
    {
        bool success = false;
        bool geometryChanged = false;
        bool verticesChanged = false;
        bool indicesChanged = false;
        bool selectionChanged = false;
        std::string error;
    };

    MeshDocument();

    const SculptMesh& mesh() const noexcept { return m_mesh; }
    const MeshSelection& selection() const noexcept { return m_selection; }
    bool isDirty() const noexcept { return !m_mesh.equals(m_savedMesh); }
    std::uint64_t revision() const noexcept { return m_revision; }

    MutationResult replaceFromText(std::string_view verticesText,
                                   std::string_view indicesText);
    MutationResult replaceMesh(SculptMesh mesh);
    MutationResult setVertex(std::size_t vertexIndex, const glm::vec3& position);
    MutationResult deleteTriangle(std::size_t triangleIndex);
    MutationResult resetToDefault();

    bool selectVertex(std::size_t vertexIndex);
    bool selectTriangle(std::size_t triangleIndex);
    void clearVertexSelection() noexcept { m_selection.clearVertex(); }
    void clearTriangleSelection() noexcept { m_selection.clearTriangle(); }
    void clearSelection() noexcept { m_selection.clear(); }
    void markSaved() { m_savedMesh = m_mesh; }

private:
    void recordGeometryChange() noexcept { ++m_revision; }
    void validateSelection() noexcept { m_selection.validateAgainst(m_mesh); }

    SculptMesh m_mesh;
    MeshSelection m_selection;
    SculptMesh m_savedMesh;
    std::uint64_t m_revision = 0;
};

} // namespace mesh_sculpt::sculpt
