#include "app/controllers/MeshSculptController.h"
#include <GLFW/glfw3.h>

namespace tools::mesh_sculpt
{

MeshSculptController::MeshSculptController(GLFWwindow* window)
    : m_window(window)
{
}

void MeshSculptController::update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy, bool cameraControl)
{
    if (!m_window || !cameraControl)
        return;

    float speed = m_orbitSpeed * dt;

    // WASD + QE movement
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

    // Free-look rotation
    camera.rotate(mouseDx * m_mouseSensitivity, mouseDy * m_mouseSensitivity);
}


} // namespace tools::mesh_sculpt
