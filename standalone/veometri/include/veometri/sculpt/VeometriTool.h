#pragma once

#include <filesystem>
#include <optional>

#include "veometri/render/Camera.h"
#include "veometri/sculpt/MeshDocument.h"
#include "veometri/sculpt/VeometriEditorUi.h"
#include "veometri/sculpt/VeometriRenderer.h"
#include "veometri/sculpt/MeshPicker.h"
#include "veometri/sculpt/VertexDragManipulator.h"

namespace veometri::sculpt {



class VeometriTool
{
public:
    explicit VeometriTool(veometri::render::Camera* camera);

    void update(float dt, bool cameraControl, bool leftClickPressed, bool deleteKeyPressed);
    void render();
    void resetMesh();
    void newDocument();
    bool openDocument(const std::filesystem::path& path);
    bool saveDocument(const std::filesystem::path& path);
    void processEditorAction(const MeshEditorAction& action);
    const SculptMesh& mesh() const noexcept { return m_document.mesh(); }
    const MeshSelection& selection() const noexcept { return m_document.selection(); }
    const veometri::render::Camera& camera() const noexcept { return *m_camera; }
    VeometriEditorUi& editorUi() noexcept { return m_editorUi; }
    const std::optional<std::filesystem::path>& currentDocumentPath() const noexcept
        { return m_currentDocumentPath; }
    bool isDirty() const noexcept { return m_document.isDirty(); }

private:
    // ---- Initialization ----
    void initializeMesh();

    // ---- Mesh Editing ----
    void synchronizeAfterDocumentChange(const MeshDocument::MutationResult& result,
                                        bool endActiveDrag);

    // ---- Picking & Dragging ----
    void beginDrag(const Ray& ray);
    void updateDrag(const Ray& ray);
    void endDrag() noexcept;

    void deleteSelectedTriangle();

private:
    veometri::render::Camera* m_camera = nullptr;

    MeshDocument m_document;
    VeometriRenderer m_renderer;

    // ---- Selection ----
    MeshPicker m_picker;

    // ---- Drag State ----
    VertexDragManipulator m_dragManipulator;
    VeometriEditorUi m_editorUi;
    std::optional<std::filesystem::path> m_currentDocumentPath;
};

} // namespace veometri::sculpt
