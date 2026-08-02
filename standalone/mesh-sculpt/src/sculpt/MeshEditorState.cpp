#include "mesh_sculpt/sculpt/MeshEditorState.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"

namespace mesh_sculpt::sculpt {

void MeshEditorState::synchronizeVertices(const SculptMesh& mesh) { m_verticesText = mesh.verticesToText(); }
void MeshEditorState::synchronizeIndices(const SculptMesh& mesh) { m_indicesText = mesh.indicesToText(); }
void MeshEditorState::synchronizeAll(const SculptMesh& mesh)
{
    synchronizeVertices(mesh);
    synchronizeIndices(mesh);
}

} // namespace mesh_sculpt::sculpt
