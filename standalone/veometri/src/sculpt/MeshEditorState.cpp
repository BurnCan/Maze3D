#include "veometri/sculpt/MeshEditorState.h"
#include "veometri/sculpt/SculptMesh.h"

namespace veometri::sculpt {

void MeshEditorState::synchronizeVertices(const SculptMesh& mesh) { m_verticesText = mesh.verticesToText(); }
void MeshEditorState::synchronizeIndices(const SculptMesh& mesh) { m_indicesText = mesh.indicesToText(); }
void MeshEditorState::synchronizeAll(const SculptMesh& mesh)
{
    synchronizeVertices(mesh);
    synchronizeIndices(mesh);
}

} // namespace veometri::sculpt
