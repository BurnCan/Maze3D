#include "tools/mesh_sculpt/MeshSculptTool.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "imgui_internal.h" // Math operators are defined here
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <cmath>

namespace tools::mesh_sculpt {

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
MeshSculptTool::MeshSculptTool(engine::Camera* camera)
    : m_camera(camera),
      m_shader(std::string(PROJECT_ROOT) + "/assets/shaders/basic.vert",
               std::string(PROJECT_ROOT) + "/assets/shaders/basic.frag"),
      m_highlightShader(std::string(PROJECT_ROOT) + "/assets/shaders/highlight.vert",
                        std::string(PROJECT_ROOT) + "/assets/shaders/highlight.frag"),
      m_selectedVertex(-1),
      m_isDragging(false),
      m_selectedTriangle (-1)
{
    initializeMesh(); // initialize default cube and text buffers
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
    return glm::normalize(m_camera->forward());
}

// ------------------------------------------------------------
// Vertex Picking
// ------------------------------------------------------------
void MeshSculptTool::pickVertex()
{
    glm::vec3 rayOrigin = getCameraRayOrigin();
    glm::vec3 rayDir = getCameraRayDirection();

    float bestDistance = 0.1f;
    int bestIndex = -1;

    auto& verts = m_mesh.vertices();

    for (int i = 0; i < static_cast<int>(verts.size()); ++i)
    {
        glm::vec3 v = verts[i];
        glm::vec3 toVertex = v - rayOrigin;
        float t = glm::dot(toVertex, rayDir);

        if (t < 0.0f)
            continue;

        glm::vec3 projected = rayOrigin + rayDir * t;
        float dist = glm::length(v - projected);

        if (dist < bestDistance)
        {
            bestDistance = dist;
            bestIndex = i;
        }
    }

    m_selectedVertex = bestIndex;
}



// ------------------------------------------------------------
// Begin Drag
// ------------------------------------------------------------
void MeshSculptTool::beginDrag()
{
    if (m_selectedVertex < 0)
        return;

    m_isDragging = true;

    // Drag plane along camera view
    m_dragPlaneNormal = getCameraRayDirection();
    m_dragStartPosition = m_mesh.vertices()[m_selectedVertex];
    m_dragPlaneDistance = -glm::dot(m_dragPlaneNormal, m_dragStartPosition);
}

// ------------------------------------------------------------
// Update Drag
// ------------------------------------------------------------
void MeshSculptTool::updateDrag()
{
    if (!m_isDragging || m_selectedVertex < 0)
        return;

    glm::vec3 rayOrigin = getCameraRayOrigin();
    glm::vec3 rayDir = getCameraRayDirection();

    float denom = glm::dot(m_dragPlaneNormal, rayDir);
    if (std::abs(denom) < 1e-5f)
        return;

    float t = -(glm::dot(m_dragPlaneNormal, rayOrigin) + m_dragPlaneDistance) / denom;
    if (t < 0.0f)
        return;

    glm::vec3 hitPoint = rayOrigin + rayDir * t;
    m_mesh.vertices()[m_selectedVertex] = hitPoint;
    m_mesh.upload(); // DynamicMesh uploads updated vertices to GPU
    syncVerticesToText();
}

// ------------------------------------------------------------
// End Drag
// ------------------------------------------------------------
void MeshSculptTool::endDrag()
{
    m_isDragging = false;
}



// ------------------------------------------------------------
// Initialize Default Cube
// ------------------------------------------------------------
void MeshSculptTool::initializeMesh()
{
    std::vector<glm::vec3> verts = {
        {-0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f,-0.5f},{ 0.5f, 0.5f,-0.5f},{-0.5f, 0.5f,-0.5f},
        {-0.5f,-0.5f, 0.5f},{ 0.5f,-0.5f, 0.5f},{ 0.5f, 0.5f, 0.5f},{-0.5f, 0.5f, 0.5f}
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
    for (auto& v : verts)
        vertStream << v.x << " " << v.y << " " << v.z << "\n";
    strncpy(m_verticesBuf, vertStream.str().c_str(), sizeof(m_verticesBuf)-1);

    std::ostringstream indStream;
    for (size_t i=0; i<indices.size(); i+=3)
        indStream << indices[i] << " " << indices[i+1] << " " << indices[i+2] << "\n";
    strncpy(m_indicesBuf, indStream.str().c_str(), sizeof(m_indicesBuf)-1);
}

// ------------------------------------------------------------
// Parse Mesh From Text
// ------------------------------------------------------------
void MeshSculptTool::parseMeshText(const char* vertsText, const char* indicesText)
{
    std::vector<glm::vec3> verts;
    std::vector<unsigned int> indices;

    std::istringstream vertStream(vertsText);
    float x,y,z;
    while (vertStream >> x >> y >> z)
        verts.emplace_back(x,y,z);

    std::istringstream indStream(indicesText);
    unsigned int a,b,c;
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

        m_selectedVertex = -1;
        m_selectedTriangle = -1;
        m_isDragging = false;
    }
}

void MeshSculptTool::syncVerticesToText()
{
    std::ostringstream vertStream;

    for (const auto& v : m_mesh.vertices())
        vertStream << v.x << " " << v.y << " " << v.z << "\n";

    std::string text = vertStream.str();

    std::memset(m_verticesBuf, 0, sizeof(m_verticesBuf));
    std::strncpy(m_verticesBuf, text.c_str(), sizeof(m_verticesBuf) - 1);
}

void MeshSculptTool::syncIndicesToText()
{
    std::ostringstream indStream;

    const auto& indices = m_mesh.indices();

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        if (i + 2 < indices.size())
        {
            indStream
                << indices[i]     << " "
                << indices[i + 1] << " "
                << indices[i + 2] << "\n";
        }
    }

    std::string text = indStream.str();

    std::memset(m_indicesBuf, 0, sizeof(m_indicesBuf));
    std::strncpy(m_indicesBuf, text.c_str(), sizeof(m_indicesBuf) - 1);
}

void MeshSculptTool::deleteSelectedTriangle()
{
    if (m_selectedTriangle < 0)
        return;

    auto& indices = m_mesh.indices();
    const size_t triBase = static_cast<size_t>(m_selectedTriangle) * 3;

    if (triBase + 2 >= indices.size())
    {
        m_selectedTriangle = -1;
        return;
    }

    indices.erase(indices.begin() + triBase,
                  indices.begin() + triBase + 3);
    m_selectedTriangle = -1;

    m_mesh.upload();
    syncIndicesToText();
}

// --- Triangle Picking ---
//int m_selectedTriangle = -1;
bool rayIntersectsTriangle(const glm::vec3& orig,
                           const glm::vec3& dir,
                           const glm::vec3& v0,
                           const glm::vec3& v1,
                           const glm::vec3& v2,
                           float& t)
{
    const float EPSILON = 0.0000001f;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(dir, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    float f = 1.0f / a;
    glm::vec3 s = orig - v0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(dir, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * glm::dot(edge2, q);
    return t > EPSILON;
}

void MeshSculptTool::pickTriangle()
{
    m_selectedTriangle = -1;

    glm::vec3 rayOrigin = m_camera->position();
    glm::vec3 rayDir = getCameraRayDirection();

    const auto& verts = m_mesh.vertices();
    const auto& indices = m_mesh.indices();

    float closestT = std::numeric_limits<float>::max();

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        const unsigned int i0 = indices[i];
        const unsigned int i1 = indices[i + 1];
        const unsigned int i2 = indices[i + 2];

        if (i0 >= verts.size() || i1 >= verts.size() || i2 >= verts.size())
            continue;

        glm::vec3 v0 = verts[i0];
        glm::vec3 v1 = verts[i1];
        glm::vec3 v2 = verts[i2];

        float t;
        if (rayIntersectsTriangle(rayOrigin, rayDir, v0, v1, v2, t))
        {
            if (t < closestT)
            {
                closestT = t;
                m_selectedTriangle = static_cast<int>(i / 3);
            }
        }
    }
}

// ------------------------------------------------------------
// Update interaction (click + drag)
// ------------------------------------------------------------
void MeshSculptTool::update(float dt, bool cameraControl, bool leftClickPressed, bool deleteKeyPressed)
{
    (void)dt;

    // If camera is controlling, ignore sculpt tool input
    if (!cameraControl)
        return;

    // --- If mouse just clicked and not dragging, select vertex ---
    if (leftClickPressed && !m_isDragging)
    {
        pickVertex();        // this preserves your original click-to-select
        beginDrag();         // begin dragging immediately after selection
    }

    // --- If mouse just clicked and not dragging, select triangle ---
    if (leftClickPressed && !m_isDragging)
    {
        pickTriangle();
    }

    // --- If currently dragging, move vertex with mouse ---
    if (m_isDragging)
        updateDrag();

    // --- If dragging but mouse released, stop dragging ---
    if (m_isDragging && !leftClickPressed)
        endDrag();

    if (m_selectedTriangle >= 0 && deleteKeyPressed)
    {
        deleteSelectedTriangle();
    }
}






// ------------------------------------------------------------
// Render the mesh and highlight selected vertex
// ------------------------------------------------------------
void MeshSculptTool::render()
{
    if (!m_camera) return;

    glm::mat4 model = glm::mat4(1.f);
    glm::mat4 view  = m_camera->view();
    glm::mat4 proj  = m_camera->projection();

    // Wireframe mesh
    m_shader.bind();
    m_shader.setMat4("uModel", model);
    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProj", proj);
    m_shader.setVec3("uColor", glm::vec3(0.7f,0.7f,0.8f));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_mesh.draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // All vertices
    glBindVertexArray(m_mesh.vao());
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(8.0f);
    m_shader.setVec3("uColor", glm::vec3(0.2f,0.9f,0.3f));
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_mesh.vertices().size()));
    glBindVertexArray(0);

    // Highlight selected vertex
    if (m_selectedVertex >= 0 && m_selectedVertex < static_cast<int>(m_mesh.vertices().size()))
    {
        m_highlightShader.bind();
        m_highlightShader.setMat4("uModel", model);
        m_highlightShader.setMat4("uView", view);
        m_highlightShader.setMat4("uProj", proj);
        m_highlightShader.setVec3("uColor", glm::vec3(1.f,0.2f,0.2f));

        glBindVertexArray(m_mesh.vao());
        glPointSize(18.0f);
        glDrawArrays(GL_POINTS, m_selectedVertex, 1);
        glBindVertexArray(0);
    }
    // Highlight selected triangle
    if (m_selectedTriangle >= 0)
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        const auto& indices = m_mesh.indices();
        const size_t triBase = static_cast<size_t>(m_selectedTriangle) * 3;

        if (triBase + 2 >= indices.size())
        {
            m_selectedTriangle = -1;
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            m_highlightShader.bind();
            m_highlightShader.setMat4("uModel", model);
            m_highlightShader.setMat4("uView", view);
            m_highlightShader.setMat4("uProj", proj);
            m_highlightShader.setVec3("uColor", glm::vec3(1.0f, 0.3f, 0.1f));

            glBindVertexArray(m_mesh.vao());

            glDrawElements(GL_TRIANGLES,
                        3,
                        GL_UNSIGNED_INT,
                        (void*)(triBase * sizeof(unsigned int)));

            glBindVertexArray(0);

            glDisable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

    }
}

void MeshSculptTool::renderOverlay(const glm::vec2& viewportMin, const glm::vec2& viewportMax, bool drawCrosshair)
{
    if (!m_camera)
        return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    const glm::vec2 viewportSize(viewportMax.x - viewportMin.x, viewportMax.y - viewportMin.y);

    if (drawCrosshair)
    {
        const glm::vec2 center(viewportMin.x + viewportSize.x * 0.5f,
                               viewportMin.y + viewportSize.y * 0.5f);
        constexpr float size = 8.0f;
        const ImU32 color = IM_COL32(255, 255, 255, 255);

        drawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size, center.y), color, 2.0f);
        drawList->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size), color, 2.0f);
    }

    if (m_selectedTriangle < 0)
        return;

    const auto& indices = m_mesh.indices();
    const auto& verts = m_mesh.vertices();
    const size_t triBase = static_cast<size_t>(m_selectedTriangle) * 3;

    if (triBase + 2 >= indices.size())
        return;

    const glm::mat4 model = glm::mat4(1.f);
    const glm::mat4 view = m_camera->view();
    const glm::mat4 proj = m_camera->projection();

    auto drawVertexIndexLabel = [&](unsigned int vertexIndex)
    {
        if (vertexIndex >= verts.size())
            return;

        const glm::vec4 clipPos = proj * view * model * glm::vec4(verts[vertexIndex], 1.0f);
        if (clipPos.w <= 0.0f)
            return;

        const glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
        if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f)
            return;

        const float screenX = viewportMin.x + (ndc.x * 0.5f + 0.5f) * viewportSize.x;
        const float screenY = viewportMin.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportSize.y;

        const std::string label = std::to_string(vertexIndex);
        const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
        const ImVec2 textPos(screenX - textSize.x * 0.5f, screenY - textSize.y - 12.0f);

        drawList->AddText(ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 255), label.c_str());
        drawList->AddText(textPos, IM_COL32(255, 255, 0, 255), label.c_str());
    };

    drawVertexIndexLabel(indices[triBase]);
    drawVertexIndexLabel(indices[triBase + 1]);
    drawVertexIndexLabel(indices[triBase + 2]);
}








// ------------------------------------------------------------
// Render ImGui panel
// ------------------------------------------------------------
void MeshSculptTool::renderImGui()
{
    ImGui::Begin("Mesh Sculpt Tool");

    auto& verts = m_mesh.vertices();
    ImGui::Text("Vertex Count: %zu", verts.size());
    ImGui::Separator();

    if (m_selectedVertex >= 0 && m_selectedVertex < static_cast<int>(verts.size()))
    {
        glm::vec3& v = verts[m_selectedVertex];
        ImGui::Text("Selected Vertex");
        ImGui::Text("Index: %d", m_selectedVertex);
        ImGui::Text("X: %.3f Y: %.3f Z: %.3f", v.x, v.y, v.z);

        if (ImGui::DragFloat3("Edit Position", &v.x, 0.01f))
            m_mesh.upload();
            syncVerticesToText();
    }
    else
    {
        ImGui::Text("No vertex selected");
    }

    ImGui::Separator();

    if (ImGui::InputTextMultiline("Vertices", m_verticesBuf, sizeof(m_verticesBuf)))
        parseMeshText(m_verticesBuf, m_indicesBuf);

    if (ImGui::InputTextMultiline("Indices", m_indicesBuf, sizeof(m_indicesBuf)))
        parseMeshText(m_verticesBuf, m_indicesBuf);
    if (m_selectedTriangle >= 0)
    {
        const auto& indices = m_mesh.indices();
        const auto& verts = m_mesh.vertices();
        const size_t triBase = static_cast<size_t>(m_selectedTriangle) * 3;

        if (triBase + 2 < indices.size())
        {
            const glm::mat4 model = glm::mat4(1.f);
            const glm::mat4 view  = m_camera->view();
            const glm::mat4 proj  = m_camera->projection();

            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            const ImVec2 displaySize = ImGui::GetIO().DisplaySize;

            auto drawVertexIndexLabel = [&](unsigned int vertexIndex)
            {
                if (vertexIndex >= verts.size())
                    return;

                const glm::vec4 clipPos = proj * view * model * glm::vec4(verts[vertexIndex], 1.0f);
                if (clipPos.w <= 0.0f)
                    return;

                const glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
                if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f)
                    return;

                const float screenX = (ndc.x * 0.5f + 0.5f) * displaySize.x;
                const float screenY = (1.0f - (ndc.y * 0.5f + 0.5f)) * displaySize.y;

                const std::string label = std::to_string(vertexIndex);
                const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
                const ImVec2 textPos(screenX - textSize.x * 0.5f, screenY - textSize.y - 12.0f);

                drawList->AddText(ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 255), label.c_str());
                drawList->AddText(textPos, IM_COL32(255, 255, 0, 255), label.c_str());
            };

            drawVertexIndexLabel(indices[triBase]);
            drawVertexIndexLabel(indices[triBase + 1]);
            drawVertexIndexLabel(indices[triBase + 2]);
        }
    }


    ImGui::End();
}

} // namespace tools::mesh_sculpt
