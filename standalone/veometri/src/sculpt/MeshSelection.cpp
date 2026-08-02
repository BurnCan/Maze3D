#include "veometri/sculpt/MeshSelection.h"

#include "veometri/sculpt/SculptMesh.h"

namespace veometri::sculpt {

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

} // namespace veometri::sculpt
