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

    void update(float dt);
    void render();
    void renderImGui();

private:
    void initializeMesh();

    // Update mesh from text input
    void parseMeshText(const char* vertsText, const char* indicesText);

    engine::Camera* m_camera = nullptr;
    engine::DynamicMesh m_mesh;      // <-- Use DynamicMesh
    engine::Shader m_shader;

    int m_selectedVertex = 0;

    // --- ImGui buffers for editable text ---
    static constexpr size_t BUF_SIZE = 8192;
    char m_verticesBuf[BUF_SIZE] = {0};
    char m_indicesBuf[BUF_SIZE] = {0};
};

} // namespace tools::mesh_sculpt
