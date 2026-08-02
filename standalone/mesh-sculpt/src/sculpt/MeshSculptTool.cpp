#include "mesh_sculpt/sculpt/MeshSculptTool.h"

namespace mesh_sculpt::sculpt {

MeshSculptTool::MeshSculptTool(render::Camera* camera) : m_camera(camera), m_renderer(camera)
{
    initializeMesh();
}

void MeshSculptTool::initializeMesh()
{
    m_sculptMesh = SculptMesh::makeDefaultCube();
    m_renderer.upload(m_sculptMesh);
    m_selection.clear();
    m_dragManipulator.end();
    m_editorUi.clearError();
    m_editorUi.synchronizeAll(m_sculptMesh);
}

void MeshSculptTool::resetMesh() { initializeMesh(); }

void MeshSculptTool::validateSelection()
{
    m_selection.validateAgainst(m_sculptMesh);
    if (!m_selection.selectedVertex())
        m_dragManipulator.end();
}

void MeshSculptTool::beginDrag(const Ray& ray)
{
    const auto selected = m_selection.selectedVertex();
    if (!selected || *selected >= m_sculptMesh.vertices().size())
    {
        m_dragManipulator.end();
        return;
    }
    m_dragManipulator.begin(m_sculptMesh.vertices()[*selected], ray.direction, ray);
}

void MeshSculptTool::updateDrag(const Ray& ray)
{
    const auto selected = m_selection.selectedVertex();
    if (!selected || *selected >= m_sculptMesh.vertices().size())
    {
        m_dragManipulator.end();
        return;
    }
    const auto position = m_dragManipulator.update(ray);
    if (!position)
        return;
    const glm::vec3 difference = *position - m_sculptMesh.vertices()[*selected];
    if (glm::dot(difference, difference) <= 1.0e-12f || !m_sculptMesh.setVertex(*selected, *position))
        return;
    m_renderer.upload(m_sculptMesh);
    m_editorUi.synchronizeVertices(m_sculptMesh);
}

void MeshSculptTool::endDrag() noexcept { m_dragManipulator.end(); }

void MeshSculptTool::deleteSelectedTriangle()
{
    const auto triangle = m_selection.selectedTriangle();
    if (!triangle)
        return;
    if (!m_sculptMesh.deleteTriangle(*triangle))
    {
        validateSelection();
        return;
    }
    m_selection.clearTriangle();
    m_renderer.upload(m_sculptMesh);
    m_editorUi.synchronizeIndices(m_sculptMesh);
}

void MeshSculptTool::processEditorAction(const MeshEditorAction& action)
{
    switch (action.type)
    {
    case MeshEditorAction::Type::ApplyMeshText:
    {
        const auto result = m_sculptMesh.replaceFromText(action.verticesText, action.indicesText);
        if (!result.success)
        {
            m_editorUi.setError(result.error);
            return;
        }
        m_selection.clear();
        m_dragManipulator.end();
        m_renderer.upload(m_sculptMesh);
        m_editorUi.clearError();
        m_editorUi.synchronizeAll(m_sculptMesh);
        break;
    }
    case MeshEditorAction::Type::UpdateSelectedVertex:
        if (const auto vertex = m_selection.selectedVertex();
            vertex && m_sculptMesh.setVertex(*vertex, action.vertexPosition))
        {
            m_renderer.upload(m_sculptMesh);
            m_editorUi.synchronizeVertices(m_sculptMesh);
        }
        break;
    case MeshEditorAction::Type::DeleteSelectedTriangle:
        deleteSelectedTriangle();
        break;
    case MeshEditorAction::Type::None:
        break;
    }
}

void MeshSculptTool::update(float, bool cameraControl, bool leftClickPressed, bool deleteKeyPressed)
{
    if (!cameraControl)
        return;
    if (leftClickPressed && !m_dragManipulator.isActive())
    {
        const auto ray = makeCameraForwardRay(*m_camera);
        if (ray)
        {
            if (const auto vertex = m_picker.pickVertex(m_sculptMesh, *ray)) m_selection.selectVertex(*vertex);
            else m_selection.clearVertex();
            beginDrag(*ray);
        }
    }
    if (leftClickPressed && !m_dragManipulator.isActive())
    {
        if (const auto ray = makeCameraForwardRay(*m_camera))
        {
            if (const auto triangle = m_picker.pickTriangle(m_sculptMesh, *ray)) m_selection.selectTriangle(*triangle);
            else m_selection.clearTriangle();
        }
    }
    if (m_dragManipulator.isActive())
    {
        if (const auto ray = makeCameraForwardRay(*m_camera)) updateDrag(*ray);
        else endDrag();
    }
    if (m_dragManipulator.isActive() && !leftClickPressed) endDrag();
    if (m_selection.selectedTriangle() && deleteKeyPressed) deleteSelectedTriangle();
}

void MeshSculptTool::render() { m_renderer.render(m_sculptMesh, m_selection); }

} // namespace mesh_sculpt::sculpt
