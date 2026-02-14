#include "engine/controllers/FPSController.h"
#include "engine/scene/FPSCamera.h"
#include <GLFW/glfw3.h>

namespace engine {

FPSController::FPSController(GLFWwindow* window)
    : m_window(window)
{
}

void FPSController::update(FPSCamera& camera, float dt, float mouseDx, float mouseDy)
{
    float vel = m_speed * dt;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveForward( vel);

    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveForward(-vel);

    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveRight(-vel);

    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight( vel);

    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.moveUp( vel);

    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.moveUp(-vel);

    camera.rotate(mouseDx, mouseDy);   // ← pure input
}

}

