#pragma once

#include "mesh_sculpt/render/Camera.h"
#include "mesh_sculpt/sculpt/MeshDocument.h"
#include "mesh_sculpt/sculpt/MeshSculptEditorUi.h"
#include "mesh_sculpt/sculpt/MeshSculptRenderer.h"
#include "mesh_sculpt/sculpt/MeshPicker.h"
#include "mesh_sculpt/sculpt/VertexDragManipulator.h"

namespace mesh_sculpt::sculpt {



class MeshSculptTool
{
public:
    explicit MeshSculptTool(mesh_sculpt::render::Camera* camera);

    void update(float dt, bool cameraControl, bool leftClickPressed, bool deleteKeyPressed);
    void render();
    void resetMesh();
    void processEditorAction(const MeshEditorAction& action);
    const SculptMesh& mesh() const noexcept { return m_document.mesh(); }
    const MeshSelection& selection() const noexcept { return m_document.selection(); }
    const mesh_sculpt::render::Camera& camera() const noexcept { return *m_camera; }
    MeshSculptEditorUi& editorUi() noexcept { return m_editorUi; }

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
    mesh_sculpt::render::Camera* m_camera = nullptr;

    MeshDocument m_document;
    MeshSculptRenderer m_renderer;

    // ---- Selection ----
    MeshPicker m_picker;

    // ---- Drag State ----
    VertexDragManipulator m_dragManipulator;
    MeshSculptEditorUi m_editorUi;
};

} // namespace mesh_sculpt::sculpt
