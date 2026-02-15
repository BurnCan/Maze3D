#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "engine/window/Window.h"
#include "engine/render/Shader.h"
#include "engine/render/CubeMesh.h"
#include "engine/render/BoxRenderer.h"
#include "engine/maze/Maze.h"
#include "engine/maze/MazeMesh.h"
#include "engine/maze/MazeCollider.h"
#include "engine/scene/FPSCamera.h"

#include "editor/EditorViewport.h"

#include <app/controllers/EditorFlyController.h>
#include <app/controllers/FPSController.h>
#include <app/controllers/ICameraController.h>

using namespace engine;

const std::filesystem::path assetRoot = MAZE3D_ASSET_ROOT;

constexpr float CELL_SIZE      = 1.0f;
constexpr float WALL_HEIGHT    = 1.0f;
constexpr float WALL_THICKNESS = 0.1f;

// Global toggles
static bool g_wireframe = false; //wireframe
static bool g_collision    = true;  //collisions

static void updateCameraWithCollision(
    EditorViewport& viewport,
    engine::FPSCamera& camera,
    engine::MazeCollider& collider,
    bool enableCollision,
    float playerRadius)
{
    // Store old position
    glm::vec3 oldPos = camera.position();

    // Let the viewport update the camera via its active controller
    viewport.begin(camera);

    // Desired new position
    glm::vec3 desired = camera.position();

    if (enableCollision)
    {
        glm::vec3 corrected = oldPos;

        // Resolve X movement
        corrected.x = desired.x;
        collider.resolve(corrected, playerRadius);

        // Resolve Z movement
        corrected.z = desired.z;
        collider.resolve(corrected, playerRadius);

        camera.setPosition(corrected);
    }
    else
    {
        camera.setPosition(desired);
    }
}



int main()
{
    try
    {
        // ---------------------------
        // Window / OpenGL setup
        // ---------------------------
        Window window(1280, 720, "Maze3D Editor");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        GLFWwindow* glfwWindow = glfwGetCurrentContext();

        // ---------------------------
        // Editor viewport + controller
        // ---------------------------
        EditorViewport viewport(glfwWindow);

        auto editorController = std::make_unique<app::EditorFlyController>(glfwWindow);
        viewport.setController(std::move(editorController));

        // ---------------------------
        // ImGui setup
        // ---------------------------
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        // ---------------------------
        // Scene framebuffer (unused, viewport handles its own)
        // ---------------------------

        // ---------------------------
        // Engine objects
        // ---------------------------
        CubeMesh cube;
        BoxRenderer boxRenderer(cube);

        Maze maze(10, 10);
        maze.generate();

        MazeMesh mazeMesh;
        mazeMesh.build(maze);

        MazeCollider collider;
        collider.build(maze);

        float mazeWidth  = maze.width()  * CELL_SIZE;
        float mazeDepth  = maze.height() * CELL_SIZE;

        // ---------------------------
        // Shaders
        // ---------------------------
        Shader wallShader(assetRoot / "shaders/wall.vert", assetRoot / "shaders/wall.frag");
        Shader floorShader(assetRoot / "shaders/floor.vert", assetRoot / "shaders/floor.frag");
        Shader ceilingShader(assetRoot / "shaders/ceiling.vert", assetRoot / "shaders/ceiling.frag");

        // ---------------------------
        // Camera
        // ---------------------------
        FPSCamera camera(60.0f, 16.f/9.f, 0.1f, 100.f);
        camera.setPosition({0.5f, 0.5f, 0.5f});

        float lastTime = (float)glfwGetTime();

        // ---------------------------
        // Render loop
        // ---------------------------
        int controllerType = 0; // 0 = EditorFly, 1 = FPS

        while (!window.shouldClose())
        {
            // ---------------------------
            // Time
            // ---------------------------
            float now = (float)glfwGetTime();
            float dt  = now - lastTime;
            lastTime = now;

            // ---------------------------
            // ImGui frame begin
            // ---------------------------
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            // ---------------------------
            // Debug window
            // ---------------------------
            ImGui::Begin("Debug");
            ImGui::Checkbox("Wireframe", &g_wireframe);

            // Collision toggle
            static bool g_collision = true;
            ImGui::Checkbox("Enable Collision", &g_collision);

            ImGui::Text("FPS: %.1f", io.Framerate);

            auto p = camera.position();
            ImGui::Text("Camera: %.2f %.2f %.2f", p.x, p.y, p.z);

            // Regenerate maze
            if (ImGui::Button("Regenerate Maze"))
            {
                maze.generate();
                mazeMesh.build(maze);
                collider.build(maze);
            }

            // Controller selection
            static int controllerIndex = 0; // 0 = EditorFly, 1 = FPS
            const char* controllerNames[] = { "EditorFly", "FPS" };
            if (ImGui::Combo("Controller", &controllerIndex, controllerNames, IM_ARRAYSIZE(controllerNames)))
            {
                // Switch controller
                switch (controllerIndex)
                {
                    case 0:
                        viewport.setController(std::make_unique<app::EditorFlyController>(glfwWindow));
                        break;
                    case 1:
                        viewport.setController(std::make_unique<app::FPSController>(glfwWindow));
                        break;
                }
            }

            ImGui::End();

            // ---------------------------
            // Store old camera position
            // ---------------------------
            glm::vec3 oldPos = camera.position();

            // ---------------------------
            // Update camera via viewport (handles controller internally)
            // ---------------------------
            constexpr float PLAYER_RADIUS = 0.25f;

            updateCameraWithCollision(
                viewport,
                camera,
                collider,
                g_collision,
                PLAYER_RADIUS
            );


            // ---------------------------
            // Render scene in viewport
            // ---------------------------
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (g_wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glDisable(GL_CULL_FACE);

            // Floor
            floorShader.bind();
            floorShader.setMat4("uView", camera.view());
            floorShader.setMat4("uProj", camera.projection());
            boxRenderer.draw(
                floorShader,
                glm::vec3(mazeWidth*0.5f, -0.05f, mazeDepth*0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth)
            );

            // Ceiling
            ceilingShader.bind();
            ceilingShader.setMat4("uView", camera.view());
            ceilingShader.setMat4("uProj", camera.projection());
            boxRenderer.draw(
                ceilingShader,
                glm::vec3(mazeWidth*0.5f, WALL_HEIGHT+0.05f, mazeDepth*0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth)
            );

            glEnable(GL_CULL_FACE);

            // Walls
            wallShader.bind();
            wallShader.setMat4("uView", camera.view());
            wallShader.setMat4("uProj", camera.projection());
            mazeMesh.draw(wallShader);

            viewport.end();

            // ---------------------------
            // ImGui render
            // ---------------------------
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup);
            }

            window.swapBuffers();
            window.pollEvents();
        }

        // ---------------------------
        // Shutdown
        // ---------------------------
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
