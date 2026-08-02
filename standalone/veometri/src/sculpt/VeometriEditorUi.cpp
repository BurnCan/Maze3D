#include "veometri/sculpt/VeometriEditorUi.h"

#include "veometri/render/Camera.h"
#include "veometri/sculpt/MeshOverlayData.h"
#include "veometri/sculpt/MeshSelection.h"
#include "veometri/sculpt/SculptMesh.h"

#include <imgui.h>

namespace veometri::sculpt {
namespace {

int resizeString(ImGuiInputTextCallbackData* data)
{
    auto& text = *static_cast<std::string*>(data->UserData);
    text.resize(static_cast<std::size_t>(data->BufTextLen));
    data->Buf = text.data();
    return 0;
}

bool inputTextMultiline(const char* label, std::string& text)
{
    if (text.capacity() == text.size())
        text.reserve(text.size() + 1);
    const bool changed = ImGui::InputTextMultiline(
        label, text.data(), text.capacity() + 1, ImVec2(0, 0),
        ImGuiInputTextFlags_CallbackResize, resizeString, &text);
    if (changed)
        text.resize(std::char_traits<char>::length(text.c_str()));
    return changed;
}

} // namespace

MeshEditorAction VeometriEditorUi::renderToolPanel(
    const SculptMesh& mesh, const MeshSelection& selection)
{
    MeshEditorAction action;
    ImGui::Begin("Veometri");
    ImGui::Text("Vertex Count: %zu", mesh.vertices().size());
    ImGui::Separator();

    if (const auto selected = selection.selectedVertex(); selected && *selected < mesh.vertices().size())
    {
        glm::vec3 position = mesh.vertices()[*selected];
        ImGui::Text("Selected Vertex");
        ImGui::Text("Index: %zu", *selected);
        ImGui::Text("X: %.3f Y: %.3f Z: %.3f", position.x, position.y, position.z);
        if (ImGui::DragFloat3("Edit Position", &position.x, 0.01f))
        {
            action.type = MeshEditorAction::Type::UpdateSelectedVertex;
            action.vertexPosition = position;
        }
    }
    else
    {
        ImGui::Text("No vertex selected");
    }

    ImGui::Separator();
    const bool verticesChanged = inputTextMultiline("Vertices", m_state.editVerticesText());
    const bool indicesChanged = inputTextMultiline("Indices", m_state.editIndicesText());
    if (verticesChanged || indicesChanged)
    {
        action.type = MeshEditorAction::Type::ApplyMeshText;
        action.verticesText = m_state.verticesText();
        action.indicesText = m_state.indicesText();
    }
    if (!m_state.errorMessage().empty())
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f), "%s", m_state.errorMessage().c_str());
    ImGui::End();
    return action;
}

void VeometriEditorUi::renderOverlay(
    const render::Camera& camera, const SculptMesh& mesh, const MeshSelection& selection,
    const glm::vec2& viewportMin, const glm::vec2& viewportMax, bool drawCrosshair) const
{
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    const glm::vec2 viewportSize = viewportMax - viewportMin;
    if (drawCrosshair && viewportSize.x > 0.0f && viewportSize.y > 0.0f)
    {
        const glm::vec2 center = viewportMin + viewportSize * 0.5f;
        constexpr float size = 8.0f;
        drawList->AddLine({center.x - size, center.y}, {center.x + size, center.y}, IM_COL32_WHITE, 2.0f);
        drawList->AddLine({center.x, center.y - size}, {center.x, center.y + size}, IM_COL32_WHITE, 2.0f);
    }
    for (const VertexLabel& item : makeMeshOverlayLabels(
             camera.view(), camera.projection(), mesh, selection, viewportMin, viewportMax))
    {
        const std::string label = std::to_string(item.vertexIndex);
        const ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
        const ImVec2 position(item.screenPosition.x - textSize.x * 0.5f,
                              item.screenPosition.y - textSize.y - 12.0f);
        drawList->AddText({position.x + 1.0f, position.y + 1.0f}, IM_COL32(0, 0, 0, 255), label.c_str());
        drawList->AddText(position, IM_COL32(255, 255, 0, 255), label.c_str());
    }
}

} // namespace veometri::sculpt
