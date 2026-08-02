#include "veometri/sculpt/VeometriTool.h"
#include "veometri/io/GeometryFileService.h"

namespace veometri::sculpt {

VeometriTool::VeometriTool(render::Camera* camera) : m_camera(camera), m_renderer(camera)
{
    initializeMesh();
}

void VeometriTool::initializeMesh()
{
    m_renderer.upload(m_document.mesh());
    m_dragManipulator.end();
    m_editorUi.clearError();
    m_editorUi.synchronizeAll(m_document.mesh());
}

void VeometriTool::resetMesh()
{
    const auto result = m_document.resetToDefault();
    synchronizeAfterDocumentChange(result, true);
    if (result.success)
        m_editorUi.clearError();
}

void VeometriTool::newDocument()
{
    const auto result = m_document.replaceMesh(SculptMesh::makeDefaultCube());
    synchronizeAfterDocumentChange(result, true);
    m_currentDocumentPath.reset();
    m_document.markSaved();
    m_editorUi.clearError();
}

bool VeometriTool::openDocument(const std::filesystem::path& path)
{
    auto loaded = io::GeometryFileService::load(path);
    if (!loaded.success)
    {
        m_editorUi.setError(loaded.error);
        return false;
    }
    const auto result = m_document.replaceMesh(std::move(loaded.mesh));
    if (!result.success)
    {
        m_editorUi.setError(result.error);
        return false;
    }
    synchronizeAfterDocumentChange(result, true);
    m_document.markSaved();
    m_currentDocumentPath = path;
    m_editorUi.clearError();
    return true;
}

bool VeometriTool::saveDocument(const std::filesystem::path& path)
{
    const auto saved = io::GeometryFileService::save(path, m_document.mesh());
    if (!saved.success)
    {
        m_editorUi.setError(saved.error);
        return false;
    }
    m_currentDocumentPath = path;
    m_document.markSaved();
    m_editorUi.clearError();
    return true;
}

void VeometriTool::synchronizeAfterDocumentChange(
    const MeshDocument::MutationResult& result, bool endActiveDrag)
{
    if (!result.success)
        return;
    if (endActiveDrag)
        m_dragManipulator.end();
    if (!result.geometryChanged)
        return;
    m_renderer.upload(m_document.mesh());
    if (result.verticesChanged)
        m_editorUi.synchronizeVertices(m_document.mesh());
    if (result.indicesChanged)
        m_editorUi.synchronizeIndices(m_document.mesh());
}

void VeometriTool::beginDrag(const Ray& ray)
{
    const auto selected = m_document.selection().selectedVertex();
    if (!selected || *selected >= m_document.mesh().vertices().size())
    {
        m_dragManipulator.end();
        return;
    }
    m_dragManipulator.begin(m_document.mesh().vertices()[*selected], ray.direction, ray);
}

void VeometriTool::updateDrag(const Ray& ray)
{
    const auto selected = m_document.selection().selectedVertex();
    if (!selected || *selected >= m_document.mesh().vertices().size())
    {
        m_dragManipulator.end();
        return;
    }
    const auto position = m_dragManipulator.update(ray);
    if (!position)
        return;
    const auto result = m_document.setVertex(*selected, *position);
    if (!result.success)
    {
        endDrag();
        return;
    }
    synchronizeAfterDocumentChange(result, false);
}

void VeometriTool::endDrag() noexcept { m_dragManipulator.end(); }

void VeometriTool::deleteSelectedTriangle()
{
    const auto triangle = m_document.selection().selectedTriangle();
    if (!triangle)
        return;
    const auto result = m_document.deleteTriangle(*triangle);
    synchronizeAfterDocumentChange(result, false);
}

void VeometriTool::processEditorAction(const MeshEditorAction& action)
{
    switch (action.type)
    {
    case MeshEditorAction::Type::ApplyMeshText:
    {
        const auto result = m_document.replaceFromText(action.verticesText, action.indicesText);
        if (!result.success)
        {
            m_editorUi.setError(result.error);
            return;
        }
        m_editorUi.clearError();
        synchronizeAfterDocumentChange(result, true);
        break;
    }
    case MeshEditorAction::Type::UpdateSelectedVertex:
        if (const auto vertex = m_document.selection().selectedVertex(); vertex)
        {
            const auto result = m_document.setVertex(*vertex, action.vertexPosition);
            if (!result.success)
                endDrag();
            else
                synchronizeAfterDocumentChange(result, false);
        }
        break;
    case MeshEditorAction::Type::DeleteSelectedTriangle:
        deleteSelectedTriangle();
        break;
    case MeshEditorAction::Type::None:
        break;
    }
}

void VeometriTool::update(float, bool cameraControl, bool leftClickPressed, bool deleteKeyPressed)
{
    if (!cameraControl)
        return;
    if (leftClickPressed && !m_dragManipulator.isActive())
    {
        const auto ray = makeCameraForwardRay(*m_camera);
        if (ray)
        {
            if (const auto vertex = m_picker.pickVertex(m_document.mesh(), *ray)) m_document.selectVertex(*vertex);
            else m_document.clearVertexSelection();
            beginDrag(*ray);
        }
    }
    if (leftClickPressed && !m_dragManipulator.isActive())
    {
        if (const auto ray = makeCameraForwardRay(*m_camera))
        {
            if (const auto triangle = m_picker.pickTriangle(m_document.mesh(), *ray)) m_document.selectTriangle(*triangle);
            else m_document.clearTriangleSelection();
        }
    }
    if (m_dragManipulator.isActive())
    {
        if (const auto ray = makeCameraForwardRay(*m_camera)) updateDrag(*ray);
        else endDrag();
    }
    if (m_dragManipulator.isActive() && !leftClickPressed) endDrag();
    if (m_document.selection().selectedTriangle() && deleteKeyPressed) deleteSelectedTriangle();
}

void VeometriTool::render() { m_renderer.render(m_document.mesh(), m_document.selection()); }

} // namespace veometri::sculpt
