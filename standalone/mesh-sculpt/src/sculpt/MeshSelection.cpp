#include "mesh_sculpt/sculpt/MeshSelection.h"

#include "mesh_sculpt/sculpt/SculptMesh.h"

namespace mesh_sculpt::sculpt {

void MeshSelection::clear() noexcept
{
    m_selectedVertex.reset();
    m_selectedTriangle.reset();
}

void MeshSelection::validateAgainst(const SculptMesh& mesh) noexcept
{
    if (m_selectedVertex && *m_selectedVertex >= mesh.vertexCount())
        m_selectedVertex.reset();
    if (m_selectedTriangle && *m_selectedTriangle >= mesh.triangleCount())
        m_selectedTriangle.reset();
}

} // namespace mesh_sculpt::sculpt
