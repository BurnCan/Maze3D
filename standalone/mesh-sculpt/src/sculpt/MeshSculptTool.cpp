#include "mesh_sculpt/sculpt/MeshSculptTool.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include "imgui_internal.h" // Math operators are defined here
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <cmath>
#include <cstring>
#include <limits>

namespace mesh_sculpt::sculpt {

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
MeshSculptTool::MeshSculptTool(mesh_sculpt::render::Camera* camera)
    : m_camera(camera),
      m_shader(std::string(MESH_SCULPT_ASSET_ROOT) + "/shaders/basic.vert",
               std::string(MESH_SCULPT_ASSET_ROOT) + "/shaders/basic.frag"),
      m_highlightShader(std::string(MESH_SCULPT_ASSET_ROOT) + "/shaders/highlight.vert",
                        std::string(MESH_SCULPT_ASSET_ROOT) + "/shaders/highlight.frag"),
      m_selectedVertex(-1),
      m_isDragging(false),
      m_selectedTriangle (-1)
{
    initializeMesh(); // initialize default cube and text buffers
}

void MeshSculptTool::resetMesh()
{
    initializeMesh();
    m_selectedVertex = -1;
    m_selectedTriangle = -1;
    m_isDragging = false;
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

    const auto& verts = m_sculptMesh.vertices();

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
    m_dragStartPosition = m_sculptMesh.vertices()[m_selectedVertex];
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
    if (!m_sculptMesh.setVertex(static_cast<std::size_t>(m_selectedVertex), hitPoint))
        return;
    uploadMeshToGpu();
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
    m_sculptMesh = SculptMesh::makeDefaultCube();
    uploadMeshToGpu();
    clearSelection();
    m_meshInputError.clear();
    syncVerticesToText();
    syncIndicesToText();
}

void MeshSculptTool::uploadMeshToGpu()
{
    m_mesh.setVertices(m_sculptMesh.vertices());
    m_mesh.setIndices(m_sculptMesh.indices());
    m_mesh.upload();
    validateSelection();
}

void MeshSculptTool::clearSelection()
{
    m_selectedVertex = -1;
    m_selectedTriangle = -1;
    m_isDragging = false;
}

void MeshSculptTool::validateSelection()
{
    if (m_selectedVertex < 0 || static_cast<std::size_t>(m_selectedVertex) >= m_sculptMesh.vertexCount())
        m_selectedVertex = -1;
    if (m_selectedTriangle < 0 || static_cast<std::size_t>(m_selectedTriangle) >= m_sculptMesh.triangleCount())
        m_selectedTriangle = -1;
    if (m_selectedVertex < 0)
        m_isDragging = false;
}

bool MeshSculptTool::copyTextToBuffer(const std::string& text, char* buffer, size_t size, const char* label)
{
    if (text.size() >= size)
    {
        m_meshInputError = std::string(label) + " text exceeds the editor buffer capacity.";
        return false;
    }
    std::memcpy(buffer, text.data(), text.size());
    buffer[text.size()] = '\0';
    return true;
}

void MeshSculptTool::applyMeshText()
{
    const SculptMesh::ParseResult result = m_sculptMesh.replaceFromText(m_verticesBuf, m_indicesBuf);
    if (!result.success)
    {
        m_meshInputError = result.error;
        return;
    }

    m_meshInputError.clear();
    clearSelection();
    uploadMeshToGpu();
}

void MeshSculptTool::syncVerticesToText()
{
    copyTextToBuffer(m_sculptMesh.verticesToText(), m_verticesBuf, sizeof(m_verticesBuf), "Vertex");
}

void MeshSculptTool::syncIndicesToText()
{
    copyTextToBuffer(m_sculptMesh.indicesToText(), m_indicesBuf, sizeof(m_indicesBuf), "Index");
}

void MeshSculptTool::deleteSelectedTriangle()
{
    if (m_selectedTriangle < 0)
        return;

    if (!m_sculptMesh.deleteTriangle(static_cast<std::size_t>(m_selectedTriangle)))
    {
        validateSelection();
        return;
    }

    m_selectedTriangle = -1;
    validateSelection();
    uploadMeshToGpu();
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

    const auto& verts = m_sculptMesh.vertices();
    const auto& indices = m_sculptMesh.indices();

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
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_sculptMesh.vertices().size()));
    glBindVertexArray(0);

    // Highlight selected vertex
    if (m_selectedVertex >= 0 && m_selectedVertex < static_cast<int>(m_sculptMesh.vertices().size()))
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
        const auto& indices = m_sculptMesh.indices();
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

    const auto& indices = m_sculptMesh.indices();
    const auto& verts = m_sculptMesh.vertices();
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

    const auto& verts = m_sculptMesh.vertices();
    ImGui::Text("Vertex Count: %zu", verts.size());
    ImGui::Separator();

    if (m_selectedVertex >= 0 && m_selectedVertex < static_cast<int>(verts.size()))
    {
        glm::vec3 v = verts[m_selectedVertex];
        ImGui::Text("Selected Vertex");
        ImGui::Text("Index: %d", m_selectedVertex);
        ImGui::Text("X: %.3f Y: %.3f Z: %.3f", v.x, v.y, v.z);

        if (ImGui::DragFloat3("Edit Position", &v.x, 0.01f) &&
            m_sculptMesh.setVertex(static_cast<std::size_t>(m_selectedVertex), v))
        {
            uploadMeshToGpu();
            syncVerticesToText();
        }
    }
    else
    {
        ImGui::Text("No vertex selected");
    }

    ImGui::Separator();

    if (ImGui::InputTextMultiline("Vertices", m_verticesBuf, sizeof(m_verticesBuf)))
        applyMeshText();

    if (ImGui::InputTextMultiline("Indices", m_indicesBuf, sizeof(m_indicesBuf)))
        applyMeshText();

    if (!m_meshInputError.empty())
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f), "%s", m_meshInputError.c_str());

    ImGui::End();
}

} // namespace mesh_sculpt::sculpt
