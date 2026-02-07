#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "engine/window/Window.h"
#include "engine/scene/FPSCamera.h"

#include "engine/render/Shader.h"
#include "engine/render/CubeMesh.h"
#include "engine/render/BoxRenderer.h"

#include "engine/maze/Maze.h"
#include "engine/maze/MazeMesh.h"
#include "engine/maze/MazeCollider.h"

#include "editor/EditorViewport.h"

using namespace engine;

const std::filesystem::path assetRoot = MAZE3D_ASSET_ROOT;

constexpr float CELL_SIZE      = 1.0f;
constexpr float WALL_HEIGHT    = 1.0f;
constexpr float WALL_THICKNESS = 0.1f;

// =====================================
// Globals
// =====================================
static bool g_wireframe = false;







// =====================================
// Main
// =====================================
int main()
{
    try
    {
        // =====================================
        // Window / OpenGL
        // =====================================
        Window window(1280, 720, "Maze3D Editor");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        GLFWwindow* glfwWindow = glfwGetCurrentContext();

        EditorViewport viewport(glfwWindow);

        // =====================================
        // ImGui init
        // =====================================
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        // =====================================
        // Scene framebuffer (editor viewport)
        // =====================================
        GLuint sceneFBO = 0;
        GLuint sceneColor = 0;
        GLuint sceneDepth = 0;

        glGenFramebuffers(1, &sceneFBO);
        glGenTextures(1, &sceneColor);
        glGenRenderbuffers(1, &sceneDepth);

        auto resizeSceneBuffer = [&](int w, int h)
        {
            if (w <= 0 || h <= 0) return;

            glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

            glBindTexture(GL_TEXTURE_2D, sceneColor);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColor, 0);

            glBindRenderbuffer(GL_RENDERBUFFER, sceneDepth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneDepth);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Scene framebuffer incomplete");

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        };

        // =====================================
        // Engine objects
        // =====================================
        engine::CubeMesh cube;
        engine::BoxRenderer boxRenderer(cube);

        engine::Maze maze(10, 10);
        maze.generate();

        engine::MazeMesh mazeMesh;
        mazeMesh.build(maze);

        engine::MazeCollider collider;
        collider.build(maze);

        float mazeWidth  = maze.width()  * CELL_SIZE;
        float mazeDepth  = maze.height() * CELL_SIZE;

        // =====================================
        // Shaders
        // =====================================
        engine::Shader wallShader(
            assetRoot / "shaders/hedge.vert",
            assetRoot / "shaders/hedge.frag");

        engine::Shader floorShader(
            assetRoot / "shaders/floor.vert",
            assetRoot / "shaders/floor.frag");

        engine::Shader ceilingShader(
            assetRoot / "shaders/ceiling.vert",
            assetRoot / "shaders/ceiling.frag");

        // =====================================
        // Camera
        // =====================================
        FPSCamera camera(60.0f, 16.f/9.f, 0.1f, 100.f);
        camera.setPosition({0.5f, 0.5f, 0.5f});

        //g_camera = &camera;

        //glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //glfwSetCursorPosCallback(glfwWindow, mouse_callback);

        float lastTime = (float)glfwGetTime();

        // =====================================
        // Render loop
        // =====================================
        while (!window.shouldClose())
        {
            float now = (float)glfwGetTime();
            float dt = now - lastTime;
            lastTime = now;

            // ==============================
            // Camera movement + collision
            // ==============================
            glm::vec3 oldPos = camera.position();
            camera.update(dt);
            glm::vec3 desired = camera.position();

            constexpr float PLAYER_RADIUS = 0.25f;
            glm::vec3 corrected = oldPos;

            corrected.x = desired.x;
            collider.resolve(corrected, PLAYER_RADIUS);
            corrected.z = desired.z;
            collider.resolve(corrected, PLAYER_RADIUS);

            camera.setPosition(corrected);

            // ==============================
            // ImGui frame begin
            // ==============================
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            // ==============================
            // Debug window
            // ==============================
            ImGui::Begin("Debug");

            ImGui::Checkbox("Wireframe", &g_wireframe);
            ImGui::Text("FPS: %.1f", io.Framerate);

            auto p = camera.position();
            ImGui::Text("Camera: %.2f %.2f %.2f", p.x, p.y, p.z);

            if (ImGui::Button("Regenerate Maze"))
            {
                maze.generate();
                mazeMesh.build(maze);
                collider.build(maze);
            }

            ImGui::End();

            // ======================================
            // Game viewport
            // ======================================
            viewport.begin(camera);

            // ---- render your scene exactly like normal ----

            if (g_wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // floor
            glDisable(GL_CULL_FACE);
            floorShader.bind();
            floorShader.setMat4("uView", camera.view());
            floorShader.setMat4("uProj", camera.projection());

            boxRenderer.draw(
                floorShader,
                glm::vec3(mazeWidth*0.5f, -0.05f, mazeDepth*0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth));

            // ceiling
            ceilingShader.bind();
            ceilingShader.setMat4("uView", camera.view());
            ceilingShader.setMat4("uProj", camera.projection());

            boxRenderer.draw(
                ceilingShader,
                glm::vec3(mazeWidth*0.5f, WALL_HEIGHT+0.05f, mazeDepth*0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth));

            glEnable(GL_CULL_FACE);

            // walls
            wallShader.bind();
            wallShader.setMat4("uView", camera.view());
            wallShader.setMat4("uProj", camera.projection());
            mazeMesh.draw(wallShader);

            // --------------------------------------
            viewport.end();

            // ==============================
            // ImGui render
            // ==============================
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

        // =====================================
        // Shutdown
        // =====================================
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
