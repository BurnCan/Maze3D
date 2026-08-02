#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "mesh_sculpt/render/Camera.h"
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
    void renderOverlay(const glm::vec2& viewportMin, const glm::vec2& viewportMax, bool drawCrosshair = true);
    void renderImGui();
    void resetMesh();

private:
    // ---- Initialization ----
    void initializeMesh();

    // ---- Mesh Editing ----
    void applyMeshText();
    void validateSelection();
    bool copyTextToBuffer(const std::string& text, char* buffer, size_t size, const char* label);

    // ---- Picking & Dragging ----
    void beginDrag(const Ray& ray);
    void updateDrag(const Ray& ray);
    void endDrag() noexcept;

    void syncVerticesToText();
    void syncIndicesToText();
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

    // ---- ImGui text buffers ----
    static constexpr size_t BUF_SIZE = 8192;
    char m_verticesBuf[BUF_SIZE] = {0};
    char m_indicesBuf[BUF_SIZE] = {0};
    std::string m_meshInputError;
};

} // namespace mesh_sculpt::sculpt
