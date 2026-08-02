#include "mesh_sculpt/sculpt/MeshDocument.h"

namespace mesh_sculpt::sculpt {

MeshDocument::MeshDocument()
    : m_mesh(SculptMesh::makeDefaultCube()), m_savedMesh(m_mesh)
{
}

MeshDocument::MutationResult MeshDocument::replaceFromText(
    std::string_view verticesText, std::string_view indicesText)
{
    SculptMesh replacement;
    const auto parseResult = replacement.replaceFromText(verticesText, indicesText);
    if (!parseResult.success)
        return {false, false, false, false, false, parseResult.error};

    return replaceMesh(std::move(replacement));
}

MeshDocument::MutationResult MeshDocument::replaceMesh(SculptMesh replacement)
{
    if (!replacement.isValid())
        return {false, false, false, false, false, replacement.validationError()};
    const bool verticesChanged = m_mesh.vertices() != replacement.vertices();
    const bool indicesChanged = m_mesh.indices() != replacement.indices();
    const bool selectionChanged = m_selection.selectedVertex().has_value() ||
                                  m_selection.selectedTriangle().has_value();
    m_selection.clear();
    if (!verticesChanged && !indicesChanged)
        return {true, false, false, false, selectionChanged, {}};

    m_mesh = std::move(replacement);
    recordGeometryChange();
    return {true, true, verticesChanged, indicesChanged, selectionChanged, {}};
}

MeshDocument::MutationResult MeshDocument::setVertex(std::size_t vertexIndex,
                                                      const glm::vec3& position)
{
    if (vertexIndex >= m_mesh.vertexCount())
        return {false, false, false, false, false, "Vertex index is out of range."};
    const auto& current = m_mesh.vertices()[vertexIndex];
    const glm::vec3 difference = position - current;
    // Preserve the drag path's existing epsilon so sub-microscopic updates do
    // not create revisions, uploads, or editor-text rewrites.
    if (difference.x * difference.x + difference.y * difference.y +
        difference.z * difference.z <= 1.0e-12f)
        return {true, false, false, false, false, {}};

    m_mesh.setVertex(vertexIndex, position);
    validateSelection();
    recordGeometryChange();
    return {true, true, true, false, false, {}};
}

MeshDocument::MutationResult MeshDocument::deleteTriangle(std::size_t triangleIndex)
{
    if (triangleIndex >= m_mesh.triangleCount())
        return {false, false, false, false, false, "Triangle index is out of range."};
    const bool selectionChanged = m_selection.selectedTriangle().has_value();
    m_mesh.deleteTriangle(triangleIndex);
    m_selection.clearTriangle();
    validateSelection();
    recordGeometryChange();
    return {true, true, false, true, selectionChanged, {}};
}

MeshDocument::MutationResult MeshDocument::resetToDefault()
{
    SculptMesh defaultMesh = SculptMesh::makeDefaultCube();
    const bool verticesChanged = m_mesh.vertices() != defaultMesh.vertices();
    const bool indicesChanged = m_mesh.indices() != defaultMesh.indices();
    const bool selectionChanged = m_selection.selectedVertex().has_value() ||
                                  m_selection.selectedTriangle().has_value();
    m_selection.clear();
    if (!verticesChanged && !indicesChanged)
        return {true, false, false, false, selectionChanged, {}};

    m_mesh = std::move(defaultMesh);
    recordGeometryChange();
    return {true, true, verticesChanged, indicesChanged, selectionChanged, {}};
}

bool MeshDocument::selectVertex(std::size_t vertexIndex)
{
    if (vertexIndex >= m_mesh.vertexCount())
        return false;
    m_selection.selectVertex(vertexIndex);
    return true;
}

bool MeshDocument::selectTriangle(std::size_t triangleIndex)
{
    if (triangleIndex >= m_mesh.triangleCount())
        return false;
    m_selection.selectTriangle(triangleIndex);
    return true;
}

} // namespace mesh_sculpt::sculpt
