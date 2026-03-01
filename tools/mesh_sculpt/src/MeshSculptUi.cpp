#include "tools/mesh_sculpt/MeshSculptUi.h"

#include "tools/mesh_sculpt/MeshSculptTool.h"

#include <imgui.h>

namespace tools::mesh_sculpt {

MeshSculptUi::FileAction MeshSculptUi::renderMainMenuBar()
{
    FileAction action = FileAction::None;

    const auto renderFileMenu = [&]()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Reset Mesh"))
                action = FileAction::ResetMesh;

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
                action = FileAction::Exit;

            ImGui::EndMenu();
        }
    };

    if (ImGui::BeginMainMenuBar())
    {
        renderFileMenu();

        ImGui::EndMainMenuBar();
    }
    else
    {
        // Fallback for setups where the global main menu bar is unavailable.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, 0.0f));

        constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_MenuBar;

        if (ImGui::Begin("MeshSculptMainMenuFallback", nullptr, flags))
        {
            if (ImGui::BeginMenuBar())
            {
                renderFileMenu();
                ImGui::EndMenuBar();
            }

            ImGui::End();
        }
    }

    return action;
}

void MeshSculptUi::renderInfoPanel(bool cameraControl) const
{
    ImGui::Begin("Camera/Editor Info");
    ImGui::Text("Press TAB to toggle camera/edit mode");
    ImGui::Text("Current mode: %s", cameraControl ? "Camera" : "Edit");
    ImGui::End();
}

void MeshSculptUi::renderToolPanel(MeshSculptTool& tool) const
{
    tool.renderImGui();
}

} // namespace tools::mesh_sculpt
