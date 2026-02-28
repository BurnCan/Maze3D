#include "engine/window/Window.h"
#include "engine/scene/FPSCamera.h"
#include "tools/mesh_sculpt/MeshSculptTool.h"
#include "app/controllers/MeshSculptController.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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
    app::MeshSculptController controller(window.nativeHandle());

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
        if (cameraControl)
            controller.update(camera, dt, mouseDx, -mouseDy);
        bool leftClick =
            glfwGetMouseButton(window.nativeHandle(),
                            GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        bool deletePressed =
            glfwGetKey(window.nativeHandle(),
                    GLFW_KEY_DELETE) == GLFW_PRESS;

        tool.update(dt, cameraControl, leftClick, deletePressed);


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

        tool.renderOverlay(glm::vec2(0.0f, 0.0f), glm::vec2(io.DisplaySize.x, io.DisplaySize.y), cameraControl);

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
