#pragma once

#include "mesh_sculpt/render/Camera.h"
#include "mesh_sculpt/sculpt/MeshSculptEditorUi.h"
#include "mesh_sculpt/sculpt/MeshSculptRenderer.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"
#include "mesh_sculpt/sculpt/MeshPicker.h"
#include "mesh_sculpt/sculpt/MeshSelection.h"
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
    const SculptMesh& mesh() const noexcept { return m_sculptMesh; }
    const MeshSelection& selection() const noexcept { return m_selection; }
    const mesh_sculpt::render::Camera& camera() const noexcept { return *m_camera; }
    MeshSculptEditorUi& editorUi() noexcept { return m_editorUi; }

private:
    // ---- Initialization ----
    void initializeMesh();

    // ---- Mesh Editing ----
    void validateSelection();

    // ---- Picking & Dragging ----
    void beginDrag(const Ray& ray);
    void updateDrag(const Ray& ray);
    void endDrag() noexcept;

    void deleteSelectedTriangle();

private:
    mesh_sculpt::render::Camera* m_camera = nullptr;

    SculptMesh m_sculptMesh;
    MeshSculptRenderer m_renderer;

    // ---- Selection ----
    MeshSelection m_selection;
    MeshPicker m_picker;

    // ---- Drag State ----
    VertexDragManipulator m_dragManipulator;
    MeshSculptEditorUi m_editorUi;
};

} // namespace mesh_sculpt::sculpt
