
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/window/Window.h"

#include "engine/scene/FPSCamera.h"
#include "engine/controllers/FPSController.h"

#include "engine/render/Shader.h"
#include "engine/render/CubeMesh.h"
#include "engine/render/BoxRenderer.h"

#include "engine/maze/Maze.h"
#include "engine/maze/MazeMesh.h"
#include "engine/maze/MazeCollider.h"

using namespace engine;

const std::filesystem::path assetRoot = MAZE3D_ASSET_ROOT;

constexpr float CELL_SIZE      = 1.0f;
constexpr float WALL_HEIGHT    = 1.0f;
constexpr float WALL_THICKNESS = 0.1f;

static bool g_wireframe = false;

int main()
{
    try
    {
        // ======================================================
        // Window / OpenGL
        // ======================================================
        Window window(1280, 720, "Maze3D");
        GLFWwindow* glfwWindow = glfwGetCurrentContext();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // ======================================================
        // Rendering helpers
        // ======================================================
        CubeMesh cube;
        BoxRenderer boxRenderer(cube);

        // ======================================================
        // Maze
        // ======================================================
        Maze maze(10, 10);
        maze.generate();

        MazeMesh mazeMesh;
        mazeMesh.build(maze);

        MazeCollider collider;
        collider.build(maze);

        float mazeWidth  = maze.width()  * CELL_SIZE;
        float mazeDepth  = maze.height() * CELL_SIZE;

        // ======================================================
        // Shaders
        // ======================================================
        Shader wallShader(
            assetRoot / "shaders/hedge.vert",
            assetRoot / "shaders/hedge.frag"
        );

        Shader floorShader(
            assetRoot / "shaders/floor.vert",
            assetRoot / "shaders/floor.frag"
        );

        Shader ceilingShader(
            assetRoot / "shaders/ceiling.vert",
            assetRoot / "shaders/ceiling.frag"
        );

        // ======================================================
        // Camera + Controller
        // ======================================================
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(glfwWindow, &fbW, &fbH);

        FPSCamera camera(
            60.0f,
            static_cast<float>(fbW) / static_cast<float>(fbH),
            0.1f,
            100.0f
        );

        FPSController controller(glfwWindow);

        camera.setPosition({ 0.5f, 0.5f, 0.5f });

        // mouse tracking (polling, not callbacks)
        double lastX = 0.0;
        double lastY = 0.0;
        glfwGetCursorPos(glfwWindow, &lastX, &lastY);

        // ======================================================
        // Timing
        // ======================================================
        float lastTime = (float)glfwGetTime();

        // ======================================================
        // Main loop
        // ======================================================
        while (!window.shouldClose())
        {
            float now = (float)glfwGetTime();
            float dt  = now - lastTime;
            lastTime  = now;

            // --------------------------------------------------
            // Poll events
            // --------------------------------------------------
            window.pollEvents();

            // --------------------------------------------------
            // Mouse delta (clean replacement for callbacks)
            // --------------------------------------------------
            double mx, my;
            glfwGetCursorPos(glfwWindow, &mx, &my);

            float dx = (float)(mx - lastX);
            float dy = (float)(lastY - my);

            lastX = mx;
            lastY = my;

            // --------------------------------------------------
            // Camera movement via controller
            // --------------------------------------------------
            glm::vec3 oldPos = camera.position();

            controller.update(camera, dt, dx, dy);

            glm::vec3 desired = camera.position();

            constexpr float PLAYER_RADIUS = 0.25f;
            glm::vec3 corrected = oldPos;

            corrected.x = desired.x;
            collider.resolve(corrected, PLAYER_RADIUS);

            corrected.z = desired.z;
            collider.resolve(corrected, PLAYER_RADIUS);

            camera.setPosition(corrected);

            // --------------------------------------------------
            // Wireframe toggle
            // --------------------------------------------------
            static bool lastState = false;
            bool pressed = glfwGetKey(glfwWindow, GLFW_KEY_F1) == GLFW_PRESS;

            if (pressed && !lastState)
                g_wireframe = !g_wireframe;

            lastState = pressed;

            if (g_wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // --------------------------------------------------
            // Rendering
            // --------------------------------------------------
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // floor
            glDisable(GL_CULL_FACE);
            floorShader.bind();
            floorShader.setMat4("uView", camera.view());
            floorShader.setMat4("uProj", camera.projection());

            boxRenderer.draw(
                floorShader,
                glm::vec3(mazeWidth * 0.5f, -0.05f, mazeDepth * 0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth)
            );

            // ceiling
            ceilingShader.bind();
            ceilingShader.setMat4("uView", camera.view());
            ceilingShader.setMat4("uProj", camera.projection());

            boxRenderer.draw(
                ceilingShader,
                glm::vec3(mazeWidth * 0.5f, WALL_HEIGHT + 0.05f, mazeDepth * 0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth)
            );

            glEnable(GL_CULL_FACE);

            // walls
            wallShader.bind();
            wallShader.setMat4("uView", camera.view());
            wallShader.setMat4("uProj", camera.projection());

            mazeMesh.draw(wallShader);

            // --------------------------------------------------
            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}

