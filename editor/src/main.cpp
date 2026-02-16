#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "engine/window/Window.h"
#include "engine/render/Shader.h"
#include "engine/render/CubeMesh.h"
#include "engine/render/CapsuleMesh.h"
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

// ---------------------------
// Constants
// ---------------------------
constexpr float CELL_SIZE      = 1.0f;
constexpr float WALL_HEIGHT    = 1.0f;
constexpr float WALL_THICKNESS = 0.1f;

constexpr float PLAYER_RADIUS     = 0.25f;
constexpr float PLAYER_HEIGHT     = 0.8f;
constexpr float PLAYER_EYE_OFFSET = 0.6f;

// ---------------------------
// Global toggles
// ---------------------------
static bool g_wireframe  = false;
static bool g_collision  = true;

enum class AppMode
{
    Editor,
    Game
};

// ---------------------------
// Camera update + collisions
// ---------------------------
static void updateCameraWithCollision(
    EditorViewport& viewport,
    FPSCamera& camera,
    MazeCollider& collider,
    bool enableCollision)
{
    glm::vec3 oldPos = camera.position();

    // Let the viewport update camera via its controller
    viewport.begin(camera);

    glm::vec3 desired = camera.position();

    if (enableCollision)
    {
        glm::vec3 baseOld = oldPos; baseOld.y = 0.0f;
        glm::vec3 baseDesired = desired; baseDesired.y = 0.0f;

        glm::vec3 corrected = baseOld;

        // Resolve X
        corrected.x = baseDesired.x;
        collider.resolve(corrected, PLAYER_RADIUS);

        // Resolve Z
        corrected.z = baseDesired.z;
        collider.resolve(corrected, PLAYER_RADIUS);

        corrected.y = PLAYER_EYE_OFFSET;
        camera.setPosition(corrected);
    }
    else
    {
        camera.setPosition(desired);
    }
}

// ---------------------------
// MAIN
// ---------------------------
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
        viewport.setController(std::make_unique<app::EditorFlyController>(glfwWindow));

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

        CapsuleMesh capsuleMesh(PLAYER_RADIUS, PLAYER_HEIGHT);

        float mazeWidth  = maze.width()  * CELL_SIZE;
        float mazeDepth  = maze.height() * CELL_SIZE;

        Shader wallShader(assetRoot / "shaders/wall.vert", assetRoot / "shaders/wall.frag");
        Shader wall2Shader(assetRoot / "shaders/wall2.vert", assetRoot / "shaders/wall2.frag"); // bricks
        Shader hedgeShader(assetRoot / "shaders/hedge.vert", assetRoot / "shaders/hedge.frag");
        Shader floorShader(assetRoot / "shaders/floor.vert", assetRoot / "shaders/floor.frag");
        Shader ceilingShader(assetRoot / "shaders/ceiling.vert", assetRoot / "shaders/ceiling.frag");
        Shader playerShader(assetRoot / "shaders/player.vert", assetRoot / "shaders/player.frag");
        Shader player2Shader(assetRoot / "shaders/player2.vert", assetRoot / "shaders/player2.frag");
        Shader player3Shader(assetRoot / "shaders/player3.vert", assetRoot / "shaders/player3.frag");

        // ---------------------------
        // Camera
        // ---------------------------
        FPSCamera camera(60.0f, 16.f/9.f, 0.1f, 100.f);
        camera.setPosition({0.5f, PLAYER_EYE_OFFSET, 0.5f});

        AppMode mode = AppMode::Editor;
        float lastTime = (float)glfwGetTime();

        // ---------------------------
        // Player tracking
        // ---------------------------
        static glm::vec3 playerPos = glm::vec3(0.5f, PLAYER_EYE_OFFSET, 0.5f);

        while (!window.shouldClose())
        {
            float now = (float)glfwGetTime();
            float dt  = now - lastTime;
            lastTime = now;

            // --- ImGui frame ---
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

            // --- Debug window ---
            ImGui::Begin("Debug");

            // Wireframe / mode toggles
            ImGui::Checkbox("Wireframe", &g_wireframe);
            bool isGameMode = (mode == AppMode::Game);
            if (ImGui::Checkbox("Game Mode", &isGameMode))
            {
                if (isGameMode)
                {
                    mode = AppMode::Game;
                    viewport.setController(std::make_unique<app::FPSController>(glfwWindow));
                    camera.setPosition({0.5f, PLAYER_EYE_OFFSET, 0.5f});
                }
                else
                {
                    mode = AppMode::Editor;
                    viewport.setController(std::make_unique<app::EditorFlyController>(glfwWindow));
                }
            }

            // Camera info
            auto camPos = camera.position();
            ImGui::Text("Camera Pos: %.2f %.2f %.2f", camPos.x, camPos.y, camPos.z);
            ImGui::Text("Camera Forward: %.2f %.2f %.2f", camera.forward().x, camera.forward().y, camera.forward().z);
            ImGui::Text("Camera Yaw: %.2f, Pitch: %.2f", camera.getYaw(), camera.getPitch());

            // Player info
            ImGui::Text("Player Eye Pos: %.2f %.2f %.2f", playerPos.x, playerPos.y, playerPos.z);

            // FPSController info
            auto* fps = dynamic_cast<app::FPSController*>(viewport.getController());
            float distance = 0.0f;
            double scrollDelta = 0.0;
            if (fps)
            {
                distance = fps->cameraDistance();
                scrollDelta = fps->scrollDelta();
            }
            ImGui::Text("Camera Distance (scroll zoom): %.2f", distance);
            ImGui::Text("Scroll Delta: %.2f", scrollDelta);

            if (ImGui::Button("Regenerate Maze"))
            {
                maze.generate();
                mazeMesh.build(maze);
                collider.build(maze);
            }

            ImGui::End();

            // --- Camera update ---
            bool collisionsEnabled = (mode == AppMode::Game);
            updateCameraWithCollision(viewport, camera, collider, collisionsEnabled);

            // Track the player position separately, NOT from camera.position()
            static glm::vec3 playerPos = glm::vec3(0.5f, 0.0f, 0.5f); // fixed base position

            // Update playerPos based on controller input
            if (mode == AppMode::Game)
            {
                auto* fps = dynamic_cast<app::FPSController*>(viewport.getController());
                if (fps)
                {
                    // Movement
                    glm::vec3 delta = fps->movementDelta(camera, dt);
                    // Move player
                    playerPos += delta;

                    // Resolve collision on player
                    if (g_collision)
                        collider.resolve(playerPos, PLAYER_RADIUS);

                    // Now compute camera AFTER player is corrected
                    //glm::vec3 playerCenter = playerPos;
                    //playerCenter.y = PLAYER_HEIGHT * 0.5f;


                    float distance = fps->cameraDistance();

                    glm::vec3 playerCenter = playerPos;
                    playerCenter.y = PLAYER_HEIGHT * 0.5f;

                    glm::vec3 camDir = glm::normalize(camera.forward());
                    glm::vec3 camPos = playerCenter - camDir * distance;

                    camera.setPosition(camPos);
                    camera.setViewMatrix(glm::lookAt(camPos, playerCenter, glm::vec3(0,1,0)));




                }
            }


            // --- Render maze ---
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (g_wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glDisable(GL_CULL_FACE);

            floorShader.bind();
            floorShader.setMat4("uView", camera.view());
            floorShader.setMat4("uProj", camera.projection());
            boxRenderer.draw(floorShader,
                            glm::vec3(mazeWidth*0.5f, -0.05f, mazeDepth*0.5f),
                            glm::vec3(mazeWidth, 0.1f, mazeDepth));

            ceilingShader.bind();
            ceilingShader.setMat4("uView", camera.view());
            ceilingShader.setMat4("uProj", camera.projection());
            boxRenderer.draw(ceilingShader,
                            glm::vec3(mazeWidth*0.5f, WALL_HEIGHT+0.05f, mazeDepth*0.5f),
                            glm::vec3(mazeWidth, 0.1f, mazeDepth));

            glEnable(GL_CULL_FACE);

            wall2Shader.bind();
            glm::mat4 model = glm::mat4(1.0f); // identity, or translate/scale as needed
            wall2Shader.setMat4("uModel", model);
            wall2Shader.setMat4("uView", camera.view());
            wall2Shader.setMat4("uProj", camera.projection());

            wall2Shader.setVec3("uCameraPos", camera.position());
            wall2Shader.setFloat("uTime", (float)glfwGetTime());

            // Optional effects
            wall2Shader.setBool("useGlow", true);
            wall2Shader.setInt("colorMode", 0); // 0=cool, 1=warm, 2=neon

            mazeMesh.draw(wall2Shader);

            //Draw player capsule
            if (mode == AppMode::Game)
            {
                glm::vec3 capsulePos = playerPos;
                capsulePos.y = PLAYER_HEIGHT * 0.5f;

                glm::mat4 model = glm::translate(glm::mat4(1.0f), capsulePos);

                player3Shader.bind();
                player3Shader.setMat4("uView", camera.view());
                player3Shader.setMat4("uProj", camera.projection());
                player3Shader.setMat4("uModel", model);
                player3Shader.setVec3("uCameraPos", camera.position());
                player3Shader.setFloat("uTime", (float)glfwGetTime());

                // Optional effects
                player3Shader.setBool("useGlow", true);
                player3Shader.setInt("colorMode", 2); // 0=cool, 1=warm, 2=neon

                capsuleMesh.draw();
            }

            viewport.end();

            // --- ImGui render ---
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
