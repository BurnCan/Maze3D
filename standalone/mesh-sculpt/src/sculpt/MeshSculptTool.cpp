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
                        std::string(MESH_SCULPT_ASSET_ROOT) + "/shaders/highlight.frag")
{
    initializeMesh(); // initialize default cube and text buffers
}

void MeshSculptTool::resetMesh()
{
    initializeMesh();
    m_selection.clear();
    m_dragManipulator.end();
}

// ------------------------------------------------------------
// Begin Drag
// ------------------------------------------------------------
void MeshSculptTool::beginDrag(const Ray& ray)
{
    const auto selectedVertex = m_selection.selectedVertex();
    if (!selectedVertex || *selectedVertex >= m_sculptMesh.vertices().size())
    {
        m_dragManipulator.end();
        return;
    }
    // Preserve the camera-facing plane chosen by the original tool.
    m_dragManipulator.begin(m_sculptMesh.vertices()[*selectedVertex], ray.direction, ray);
}

// ------------------------------------------------------------
// Update Drag
// ------------------------------------------------------------
void MeshSculptTool::updateDrag(const Ray& ray)
{
    const auto selectedVertex = m_selection.selectedVertex();
    if (!selectedVertex || *selectedVertex >= m_sculptMesh.vertices().size())
    {
        m_dragManipulator.end();
        return;
    }
    const auto position = m_dragManipulator.update(ray);
    if (!position)
        return;
    const glm::vec3 difference = *position - m_sculptMesh.vertices()[*selectedVertex];
    if (glm::dot(difference, difference) <= 1.0e-12f)
        return;
    if (!m_sculptMesh.setVertex(*selectedVertex, *position))
        return;
    uploadMeshToGpu();
    syncVerticesToText();
}

// ------------------------------------------------------------
// End Drag
// ------------------------------------------------------------
void MeshSculptTool::endDrag() noexcept
{
    m_dragManipulator.end();
}



// ------------------------------------------------------------
// Initialize Default Cube
// ------------------------------------------------------------
void MeshSculptTool::initializeMesh()
{
    m_sculptMesh = SculptMesh::makeDefaultCube();
    uploadMeshToGpu();
    m_selection.clear();
    m_dragManipulator.end();
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

void MeshSculptTool::validateSelection()
{
    m_selection.validateAgainst(m_sculptMesh);
    if (!m_selection.selectedVertex())
        m_dragManipulator.end();
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
    m_selection.clear();
    m_dragManipulator.end();
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
    const auto selectedTriangle = m_selection.selectedTriangle();
    if (!selectedTriangle)
        return;

    if (!m_sculptMesh.deleteTriangle(*selectedTriangle))
    {
        validateSelection();
        return;
    }

    m_selection.clearTriangle();
    uploadMeshToGpu();
    syncIndicesToText();
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
    if (leftClickPressed && !m_dragManipulator.isActive())
    {
        const auto ray = makeCameraForwardRay(*m_camera);
        if (ray)
        {
            const auto vertex = m_picker.pickVertex(m_sculptMesh, *ray);
            if (vertex)
                m_selection.selectVertex(*vertex);
            else
                m_selection.clearVertex();
        }
        if (ray)
            beginDrag(*ray);
    }

    // --- If mouse just clicked and not dragging, select triangle ---
    if (leftClickPressed && !m_dragManipulator.isActive())
    {
        const auto ray = makeCameraForwardRay(*m_camera);
        if (ray)
        {
            const auto triangle = m_picker.pickTriangle(m_sculptMesh, *ray);
            if (triangle)
                m_selection.selectTriangle(*triangle);
            else
                m_selection.clearTriangle();
        }
    }

    // --- If currently dragging, move vertex with mouse ---
    if (m_dragManipulator.isActive())
    {
        if (const auto ray = makeCameraForwardRay(*m_camera))
            updateDrag(*ray);
        else
            endDrag();
    }

    // --- If dragging but mouse released, stop dragging ---
    if (m_dragManipulator.isActive() && !leftClickPressed)
        endDrag();

    if (m_selection.selectedTriangle() && deleteKeyPressed)
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
    if (const auto selectedVertex = m_selection.selectedVertex(); selectedVertex && *selectedVertex < m_sculptMesh.vertices().size())
    {
        m_highlightShader.bind();
        m_highlightShader.setMat4("uModel", model);
        m_highlightShader.setMat4("uView", view);
        m_highlightShader.setMat4("uProj", proj);
        m_highlightShader.setVec3("uColor", glm::vec3(1.f,0.2f,0.2f));

        glBindVertexArray(m_mesh.vao());
        glPointSize(18.0f);
        glDrawArrays(GL_POINTS, static_cast<GLint>(*selectedVertex), 1);
        glBindVertexArray(0);
    }
    // Highlight selected triangle
    if (const auto selectedTriangle = m_selection.selectedTriangle())
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        const auto& indices = m_sculptMesh.indices();
        const size_t triBase = *selectedTriangle * 3;

        if (triBase + 2 >= indices.size())
        {
            m_selection.clearTriangle();
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

    const auto selectedTriangle = m_selection.selectedTriangle();
    if (!selectedTriangle)
        return;

    const auto& indices = m_sculptMesh.indices();
    const auto& verts = m_sculptMesh.vertices();
    const size_t triBase = *selectedTriangle * 3;

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

    if (const auto selectedVertex = m_selection.selectedVertex(); selectedVertex && *selectedVertex < verts.size())
    {
        glm::vec3 v = verts[*selectedVertex];
        ImGui::Text("Selected Vertex");
        ImGui::Text("Index: %zu", *selectedVertex);
        ImGui::Text("X: %.3f Y: %.3f Z: %.3f", v.x, v.y, v.z);

        if (ImGui::DragFloat3("Edit Position", &v.x, 0.01f) &&
            m_sculptMesh.setVertex(*selectedVertex, v))
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
