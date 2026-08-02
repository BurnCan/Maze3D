#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "mesh_sculpt/render/Shader.h"
#include "mesh_sculpt/render/DynamicMesh.h"
#include "mesh_sculpt/render/Camera.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"

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
    void uploadMeshToGpu();
    void clearSelection();
    void validateSelection();
    bool copyTextToBuffer(const std::string& text, char* buffer, size_t size, const char* label);

    // ---- Picking & Dragging ----
    void pickVertex();
    void beginDrag();
    void updateDrag();
    void endDrag();

    void syncVerticesToText();
    void syncIndicesToText();
    void pickTriangle();
    void deleteSelectedTriangle();

    glm::vec3 getCameraRayOrigin() const;
    glm::vec3 getCameraRayDirection() const;

private:
    mesh_sculpt::render::Camera* m_camera = nullptr;

    mesh_sculpt::render::DynamicMesh m_mesh;
    SculptMesh m_sculptMesh;
    mesh_sculpt::render::Shader m_shader;
    mesh_sculpt::render::Shader m_highlightShader;

    // ---- Selection ----
    int  m_selectedVertex = -1;
    int  m_selectedTriangle = -1;

    // ---- Drag State ----
    bool m_isDragging = false;
    glm::vec3 m_dragPlaneNormal = glm::vec3(0.0f);
    glm::vec3 m_dragStartPosition = glm::vec3(0.0f);
    float m_dragPlaneDistance = 0.0f;

    // ---- ImGui text buffers ----
    static constexpr size_t BUF_SIZE = 8192;
    char m_verticesBuf[BUF_SIZE] = {0};
    char m_indicesBuf[BUF_SIZE] = {0};
    std::string m_meshInputError;
};

} // namespace mesh_sculpt::sculpt
