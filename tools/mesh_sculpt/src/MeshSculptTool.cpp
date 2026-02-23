#include "tools/mesh_sculpt/MeshSculptTool.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <string>
#include <algorithm>

namespace tools::mesh_sculpt {

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
MeshSculptTool::MeshSculptTool(engine::Camera* camera)
    : m_camera(camera),
      m_shader(std::string(PROJECT_ROOT) + "/assets/shaders/basic.vert",
               std::string(PROJECT_ROOT) + "/assets/shaders/basic.frag"),
      m_highlightShader (std::string(PROJECT_ROOT) + "/assets/shaders/highlight.vert",
                         std::string(PROJECT_ROOT) + "/assets/shaders/highlight.frag"),
      m_selectedVertex(-1)
{
    initializeMesh();
}

// ------------------------------------------------------------
// Camera Ray Helpers
// ------------------------------------------------------------
glm::vec3 MeshSculptTool::getCameraRayOrigin() const
{
    return m_camera->position();
}

glm::vec3 MeshSculptTool::getCameraRayDirection() const
{
    return m_camera->forward(); // must be normalized
}

// ------------------------------------------------------------
// Initialize Default Cube
// ------------------------------------------------------------
void MeshSculptTool::initializeMesh()
{
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
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        0,4,7, 7,3,0,
        1,5,6, 6,2,1,
        3,7,6, 6,2,3,
        0,1,5, 5,4,0
    };

    m_mesh.setVertices(verts);
    m_mesh.setIndices(indices);
    m_mesh.upload();

    // Fill ImGui text buffers
    std::ostringstream vertStream;
    for (const auto& v : verts)
        vertStream << v.x << " " << v.y << " " << v.z << "\n";

    strncpy(m_verticesBuf, vertStream.str().c_str(), sizeof(m_verticesBuf) - 1);

    std::ostringstream indStream;
    for (size_t i = 0; i < indices.size(); i += 3)
        indStream << indices[i] << " " << indices[i+1] << " " << indices[i+2] << "\n";

    strncpy(m_indicesBuf, indStream.str().c_str(), sizeof(m_indicesBuf) - 1);
}

// ------------------------------------------------------------
// Parse Mesh From Text
// ------------------------------------------------------------
void MeshSculptTool::parseMeshText(const char* vertsText,
                                   const char* indicesText)
{
    std::vector<glm::vec3> verts;
    std::vector<unsigned int> indices;

    std::istringstream vertStream(vertsText);
    float x, y, z;

    while (vertStream >> x >> y >> z)
        verts.emplace_back(x, y, z);

    std::istringstream indStream(indicesText);
    unsigned int a, b, c;

    while (indStream >> a >> b >> c)
    {
        indices.push_back(a);
        indices.push_back(b);
        indices.push_back(c);
    }

    if (!verts.empty() && !indices.empty())
    {
        m_mesh.setVertices(verts);
        m_mesh.setIndices(indices);
        m_mesh.upload();
    }
}

// ------------------------------------------------------------
// Update (Selection Logic)
// ------------------------------------------------------------
void MeshSculptTool::update(float dt,
                            bool cameraControl,
                            bool leftClickPressed)
{
    (void)dt;

    if (!cameraControl || !leftClickPressed)
        return;

    glm::vec3 rayOrigin = getCameraRayOrigin();
    glm::vec3 rayDir    = getCameraRayDirection();

    float bestDistance = 0.1f;
    int bestIndex = -1;

    auto& verts = m_mesh.vertices();

    for (size_t i = 0; i < verts.size(); ++i)
    {
        glm::vec3 v = verts[i];

        glm::vec3 toPoint = v - rayOrigin;
        float t = glm::dot(toPoint, rayDir);

        if (t < 0.0f)
            continue;

        glm::vec3 closestPoint = rayOrigin + rayDir * t;
        float dist = glm::length(v - closestPoint);

        if (dist < bestDistance)
        {
            bestDistance = dist;
            bestIndex = static_cast<int>(i);
        }
    }

    if (bestIndex >= 0)
        m_selectedVertex = bestIndex;
}

// ------------------------------------------------------------
// Render the mesh and highlight the selected vertex
// ------------------------------------------------------------
void MeshSculptTool::render()
{
    if (!m_camera)
        return;

    // ------------------------------
    // Draw the main mesh (wireframe)
    // ------------------------------
    m_shader.bind();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view  = m_camera->view();
    glm::mat4 proj  = m_camera->projection();

    m_shader.setMat4("uModel", model);
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProj", proj);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Wireframe color
    m_shader.setVec3("uColor", glm::vec3(0.7f, 0.7f, 0.8f));
    m_mesh.draw();

    // ------------------------------
    // Draw all vertices as points
    // ------------------------------
    glBindVertexArray(m_mesh.vao());
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(8.0f);

    m_shader.setVec3("uColor", glm::vec3(0.1f, 0.9f, 0.2f));
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_mesh.vertices().size()));
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ------------------------------
    // Draw selected vertex (highlight)
    // ------------------------------
    if (m_selectedVertex >= 0 &&
        m_selectedVertex < static_cast<int>(m_mesh.vertices().size()))
    {
        m_highlightShader.bind();

        // Pass same matrices to highlight shader
        m_highlightShader.setMat4("uModel", model);
        m_highlightShader.setMat4("uView", view);
        m_highlightShader.setMat4("uProj", proj);

        glBindVertexArray(m_mesh.vao());
        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(18.0f); // larger for highlight

        // Highlight color
        m_highlightShader.setVec3("uColor", glm::vec3(1.0f, 0.2f, 0.2f));

        glDrawArrays(GL_POINTS, m_selectedVertex, 1);

        glBindVertexArray(0);
    }
}


// ------------------------------------------------------------
// Render ImGui panel for editing vertices
// ------------------------------------------------------------
void MeshSculptTool::renderImGui()
{
    ImGui::Begin("Mesh Sculpt Tool");

    auto& verts = m_mesh.vertices();

    ImGui::Text("Vertex Count: %zu", verts.size());
    ImGui::Separator();

    // Selected vertex info + editable position
    if (m_selectedVertex >= 0 &&
        m_selectedVertex < static_cast<int>(verts.size()))
    {
        glm::vec3& v = verts[m_selectedVertex];

        ImGui::Text("Selected Vertex");
        ImGui::Text("Index: %d", m_selectedVertex);
        ImGui::Text("X: %.3f", v.x);
        ImGui::Text("Y: %.3f", v.y);
        ImGui::Text("Z: %.3f", v.z);

        if (ImGui::DragFloat3("Edit Position", &v.x, 0.01f))
        {
            // Update mesh buffer
            m_mesh.upload();
        }
    }
    else
    {
        ImGui::Text("No vertex selected");
    }

    ImGui::Separator();

    // Raw text editing
    if (ImGui::InputTextMultiline("Vertices", m_verticesBuf, sizeof(m_verticesBuf)))
    {
        parseMeshText(m_verticesBuf, m_indicesBuf);
    }

    if (ImGui::InputTextMultiline("Indices", m_indicesBuf, sizeof(m_indicesBuf)))
    {
        parseMeshText(m_verticesBuf, m_indicesBuf);
    }

    ImGui::End();
}


} // namespace tools::mesh_sculpt
