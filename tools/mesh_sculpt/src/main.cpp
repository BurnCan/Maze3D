#include "engine/window/Window.h"
#include "engine/scene/FPSCamera.h"
#include "tools/mesh_sculpt/MeshSculptTool.h"
#include "app/controllers/MeshSculptController.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

int main()
{
    engine::Window window(1280, 720, "Mesh Sculpt Tool");
    glEnable(GL_DEPTH_TEST);

    // --- Initialize ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 3.2f;

    ImGui_ImplGlfw_InitForOpenGL(window.nativeHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    engine::FPSCamera camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
    tools::mesh_sculpt::MeshSculptTool tool(&camera);
    tools::mesh_sculpt::MeshSculptController controller(window.nativeHandle());

    // --- Timing ---
    float lastTime = static_cast<float>(glfwGetTime());

    // --- Mouse tracking ---
    double lastMouseX, lastMouseY;
    glfwGetCursorPos(window.nativeHandle(), &lastMouseX, &lastMouseY);

    // --- Camera/Edit toggle ---
    bool cameraControl = true;
    bool tabPressedLastFrame = false;

    glfwSetInputMode(window.nativeHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!window.shouldClose())
    {
        // --- Delta time ---
        float currentTime = static_cast<float>(glfwGetTime());
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        window.pollEvents();

        // --- TAB toggle (edge-triggered) ---
        bool tabPressedNow =
            glfwGetKey(window.nativeHandle(), GLFW_KEY_TAB) == GLFW_PRESS;

        if (tabPressedNow && !tabPressedLastFrame)
        {
            cameraControl = !cameraControl;

            if (cameraControl)
            {
                glfwSetInputMode(window.nativeHandle(),
                                 GLFW_CURSOR,
                                 GLFW_CURSOR_DISABLED);

                // Reset mouse delta to avoid jump
                glfwGetCursorPos(window.nativeHandle(), &lastMouseX, &lastMouseY);
            }
            else
            {
                glfwSetInputMode(window.nativeHandle(),
                                 GLFW_CURSOR,
                                 GLFW_CURSOR_NORMAL);
            }
        }

        tabPressedLastFrame = tabPressedNow;

        // --- Mouse delta ---
        double mouseX, mouseY;
        glfwGetCursorPos(window.nativeHandle(), &mouseX, &mouseY);

        float mouseDx = static_cast<float>(mouseX - lastMouseX);
        float mouseDy = static_cast<float>(mouseY - lastMouseY);

        lastMouseX = mouseX;
        lastMouseY = mouseY;

        // --- Update ---
        controller.update(camera, dt, mouseDx, -mouseDy, cameraControl);
        bool leftClick =
            glfwGetMouseButton(window.nativeHandle(),
                            GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        tool.update(dt, cameraControl, leftClick);


        // --- Render Scene ---
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tool.render();

        // --- ImGui Frame ---
        ImGuiIO& io = ImGui::GetIO();

        if (cameraControl)
            io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
        else
            io.ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;

        // --- ImGui Frame ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Crosshair ---
        if (cameraControl)
        {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();

            ImVec2 center(
                ImGui::GetIO().DisplaySize.x * 0.5f,
                ImGui::GetIO().DisplaySize.y * 0.5f
            );

            float size = 8.0f;
            ImU32 color = IM_COL32(255, 255, 255, 255);

            drawList->AddLine(
                ImVec2(center.x - size, center.y),
                ImVec2(center.x + size, center.y),
                color, 2.0f);

            drawList->AddLine(
                ImVec2(center.x, center.y - size),
                ImVec2(center.x, center.y + size),
                color, 2.0f);
        }


        tool.renderImGui();

        ImGui::Begin("Camera/Editor Info");
        ImGui::Text("Press TAB to toggle camera/edit mode");
        ImGui::Text("Current mode: %s", cameraControl ? "Camera" : "Edit");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swapBuffers();
    }

    // --- Shutdown ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
