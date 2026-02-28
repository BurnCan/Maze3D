#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "engine/render/Shader.h"
#include "engine/render/DynamicMesh.h"
#include "engine/scene/Camera.h"

namespace tools::mesh_sculpt {

class MeshSculptTool
{
public:
    explicit MeshSculptTool(engine::Camera* camera);

    void update(float dt, bool cameraControl, bool leftClickPressed, bool deleteKeyPressed);
    void render();
    void renderOverlay(const glm::vec2& viewportMin, const glm::vec2& viewportMax, bool drawCrosshair = true);
    void renderImGui();

private:
    // ---- Initialization ----
    void initializeMesh();

    // ---- Mesh Editing ----
    void parseMeshText(const char* vertsText, const char* indicesText);

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
    engine::Camera* m_camera = nullptr;

    engine::DynamicMesh m_mesh;
    engine::Shader m_shader;
    engine::Shader m_highlightShader;

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
};

} // namespace tools::mesh_sculpt
