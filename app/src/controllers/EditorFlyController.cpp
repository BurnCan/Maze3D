#include "app/controllers/EditorFlyController.h"
#include <GLFW/glfw3.h>

namespace app
{

EditorFlyController::EditorFlyController(GLFWwindow* window)
    : m_window(window)
{
}

void EditorFlyController::update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy)
{
    if (!m_window) return;

    float speed = m_baseSpeed * dt;

    // shift speed boost
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        speed *= 3.0f;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveForward(speed);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveForward(-speed);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveRight(-speed);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(speed);
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.moveUp(-speed);
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS)
        camera.moveUp(speed);

    // mouse look
    camera.rotate(mouseDx, mouseDy);
}

} // namespace app
