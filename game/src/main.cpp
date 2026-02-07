#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

#include "engine/window/Window.h"

#include "engine/scene/FPSCamera.h"

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

//Wireframe mode toggle
static bool g_wireframe = false;

// --------------------
// Mouse handling
// --------------------
static bool firstMouse = true;
static double lastX = 0.0;
static double lastY = 0.0;
static FPSCamera* g_camera = nullptr;

static void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double dx = xpos - lastX;
    double dy = lastY - ypos; // inverted Y

    lastX = xpos;
    lastY = ypos;

    if (g_camera) {
        g_camera->onMouseMove((float)dx, (float)dy);
    }
}

int main()
{


    try {



        // --------------------
        // Window / OpenGL
        // --------------------
        Window window(1280, 720, "Maze3D");
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);



        //declare CubeMesh cube
        engine::CubeMesh cube;
        engine::BoxRenderer boxRenderer(cube);

        // --------------------
        // Maze (still used for collision)
        // --------------------
        engine::Maze maze(10, 10);
        maze.generate();

        engine::MazeMesh mazeMesh;
        mazeMesh.build(maze);

        engine::MazeCollider collider;
        collider.build(maze);

        float mazeWidth  = maze.width()  * CELL_SIZE;
        float mazeDepth  = maze.height() * CELL_SIZE;







        // --------------------
        // Shader sources
        // --------------------
        engine::Shader shader(
            assetRoot / "shaders/basic.vert",
            assetRoot / "shaders/basic.frag"
        );

        engine::Shader wallShader(
            assetRoot / "shaders/hedge.vert",
            assetRoot / "shaders/hedge.frag"
        );

        engine::Shader floorShader(
            assetRoot / "shaders/floor.vert",
            assetRoot / "shaders/floor.frag"
        );

        engine::Shader ceilingShader(
            assetRoot / "shaders/ceiling.vert",
            assetRoot / "shaders/ceiling.frag"
        );



        // --------------------
        // Camera
        // --------------------
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);

        FPSCamera camera(
            60.0f,
            static_cast<float>(fbW) / static_cast<float>(fbH),
            0.1f,
            100.0f
        );

        // 🔑 IMPORTANT: deterministic spawn
        //original
        //camera.setPosition({ 0.0f, 0.5f, 2.0f });
        //spawn upgrade?
        //spawn in center of a known reachable cell

        int sx = 0;
        int sy = 0;

        camera.setPosition({
            (sx + 0.5f) * CELL_SIZE,
            0.5f,
            (sy + 0.5f) * CELL_SIZE
        });
        //debug print
        //std::cout << "Spawn: " << camera.position().x
          //<< ", " << camera.position().z << "\n";

        g_camera = &camera;

        GLFWwindow* glfwWindow = glfwGetCurrentContext();
        glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(glfwWindow, mouse_callback);

        // --------------------
        // Timing
        // --------------------
        float lastTime = (float)glfwGetTime();

        // --------------------
        // Render loop
        // --------------------
        while (!window.shouldClose()) {
            float now = (float)glfwGetTime();
            float deltaTime = now - lastTime;
            lastTime = now;

            // ---- camera movement
            glm::vec3 oldPos = camera.position();
            camera.update(deltaTime);
            glm::vec3 desired = camera.position();

            constexpr float PLAYER_RADIUS = 0.25f;
            glm::vec3 corrected = oldPos;

            corrected.x = desired.x;
            collider.resolve(corrected, PLAYER_RADIUS);

            corrected.z = desired.z;
            collider.resolve(corrected, PLAYER_RADIUS);

            camera.setPosition(corrected);

            // ---- rendering
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            floorShader.bind();
            floorShader.setMat4("uView", camera.view());
            floorShader.setMat4("uProj", camera.projection());
            floorShader.setVec3("uColor", glm::vec3(0.9f, 0.3f, 0.2f));

            //Wireframe
            static bool lastState = false;
            bool pressed = glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_F1) == GLFW_PRESS;

            if (pressed && !lastState) {
                g_wireframe = !g_wireframe;
            }

            lastState = pressed;

            if (g_wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


            // TODO: Replace BoxRenderer floor/ceiling with PlaneMesh (proper winding)
            glDisable(GL_CULL_FACE);
            //Draw floor
            boxRenderer.draw(
                floorShader,
                glm::vec3(mazeWidth * 0.5f, -0.05f, mazeDepth * 0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth)

            );

            ceilingShader.bind();
            ceilingShader.setMat4("uView", camera.view());
            ceilingShader.setMat4("uProj", camera.projection());
            ceilingShader.setVec3("uColor", glm::vec3(0.9f, 0.3f, 0.2f));

            //Draw ceiling
            boxRenderer.draw(
                ceilingShader,
                glm::vec3(mazeWidth * 0.5f, WALL_HEIGHT + 0.05f, mazeDepth * 0.5f),
                glm::vec3(mazeWidth, 0.1f, mazeDepth)

            );
            glEnable(GL_CULL_FACE);



            // Draw walls
            // ---- draw maze walls (batched)
            wallShader.bind();
            wallShader.setMat4("uView", camera.view());
            wallShader.setMat4("uProj", camera.projection());
            wallShader.setVec3("uColor", glm::vec3(0.7f, 0.7f, 0.75f)); // if used

            mazeMesh.draw(wallShader);


            window.swapBuffers();
            window.pollEvents();
        }


    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}


