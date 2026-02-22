#include "tools/mesh_sculpt/MeshSculptTool.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <string>
#include <algorithm>

namespace tools::mesh_sculpt {

// -------------------- Constructor --------------------
MeshSculptTool::MeshSculptTool(engine::Camera* camera)
    : m_camera(camera),
      m_shader(std::string(PROJECT_ROOT) + "/assets/shaders/basic.vert",
               std::string(PROJECT_ROOT) + "/assets/shaders/basic.frag"),
      m_selectedVertex(0)
{
    initializeMesh();
}

// -------------------- Initialize Default Mesh --------------------
void MeshSculptTool::initializeMesh()
{
    // Start with a simple cube
    std::vector<glm::vec3> verts = {
        {-0.5f,-0.5f,-0.5f},
        { 0.5f,-0.5f,-0.5f},
        { 0.5f, 0.5f,-0.5f},
        {-0.5f, 0.5f,-0.5f},
        {-0.5f,-0.5f, 0.5f},
        { 0.5f,-0.5f, 0.5f},
        { 0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f}
    };

    std::vector<unsigned int> indices = {
        0,1,2, 2,3,0,    // Back
        4,5,6, 6,7,4,    // Front
        0,4,7, 7,3,0,    // Left
        1,5,6, 6,2,1,    // Right
        3,7,6, 6,2,3,    // Top
        0,1,5, 5,4,0     // Bottom
    };

    m_mesh.setVertices(verts);
    m_mesh.setIndices(indices);
    m_mesh.upload();

    // Fill ImGui buffers for editing
    std::ostringstream vertStream;
    for (const auto& v : verts)
        vertStream << v.x << " " << v.y << " " << v.z << "\n";
    strncpy(m_verticesBuf, vertStream.str().c_str(), sizeof(m_verticesBuf) - 1);

    std::ostringstream indStream;
    for (size_t i = 0; i < indices.size(); i += 3)
        indStream << indices[i] << " " << indices[i+1] << " " << indices[i+2] << "\n";
    strncpy(m_indicesBuf, indStream.str().c_str(), sizeof(m_indicesBuf) - 1);
}

// -------------------- Update mesh from text --------------------
void MeshSculptTool::parseMeshText(const char* vertsText, const char* indicesText)
{
    std::vector<glm::vec3> verts;
    std::vector<unsigned int> indices;

    // Parse vertices
    std::istringstream vertStream(vertsText);
    float x, y, z;
    while (vertStream >> x >> y >> z)
        verts.emplace_back(x, y, z);

    // Parse indices
    std::istringstream indStream(indicesText);
    unsigned int a, b, c;
    while (indStream >> a >> b >> c)
        indices.push_back(a), indices.push_back(b), indices.push_back(c);

    if (!verts.empty() && !indices.empty())
    {
        m_mesh.setVertices(verts);
        m_mesh.setIndices(indices);
        m_mesh.upload();
    }
}

// -------------------- Update (per-frame) --------------------
void MeshSculptTool::update(float dt)
{
    (void)dt;
}

// -------------------- Render --------------------
void MeshSculptTool::render()
{
    if (!m_camera)
        return;

    m_shader.bind();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = m_camera->view();
    glm::mat4 proj = m_camera->projection();
    glm::mat4 viewProj = proj * view;

    m_shader.setMat4("uModel", model);
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProj", proj);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // ---- Render Solid Mesh ----
    m_shader.setVec3("uColor", glm::vec3(0.7f, 0.7f, 0.8f));
    m_mesh.draw();

    // ---- Render Vertex Points ----
    glBindVertexArray(m_mesh.vao());
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(8.0f);

    m_shader.setVec3("uColor", glm::vec3(0.1f, 0.9f, 0.2f));
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_mesh.vertices().size()));

    // ---- Highlight Selected Vertex ----
    if (!m_mesh.vertices().empty())
    {
        glPointSize(14.0f);
        m_shader.setVec3("uColor", glm::vec3(1.0f, 0.2f, 0.2f));
        glDrawArrays(GL_POINTS, m_selectedVertex, 1);
    }

    glBindVertexArray(0);
}

// -------------------- ImGui Panel --------------------
void MeshSculptTool::renderImGui()
{
    ImGui::Begin("Mesh Sculpt Tool");

    auto& verts = m_mesh.vertices();
    if (!verts.empty())
    {
        // Safety clamp
        m_selectedVertex = std::clamp(m_selectedVertex, 0, static_cast<int>(verts.size()) - 1);

        ImGui::SliderInt("Vertex", &m_selectedVertex, 0, static_cast<int>(verts.size()) - 1);
        glm::vec3& v = verts[m_selectedVertex];
        if (ImGui::DragFloat3("Position", &v.x, 0.01f))
            m_mesh.upload();
    }

    ImGui::Text("Vertex Count: %zu", verts.size());

    // Editable vertices and indices
    if (ImGui::InputTextMultiline("Vertices", m_verticesBuf, sizeof(m_verticesBuf)))
        parseMeshText(m_verticesBuf, m_indicesBuf);

    if (ImGui::InputTextMultiline("Indices", m_indicesBuf, sizeof(m_indicesBuf)))
        parseMeshText(m_verticesBuf, m_indicesBuf);

    ImGui::End();
}

} // namespace tools::mesh_sculpt
