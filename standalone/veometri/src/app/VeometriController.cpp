#include "veometri/app/VeometriController.h"

namespace veometri::app
{

VeometriController::VeometriController(GLFWwindow* window)
    : m_window(window)
{
}

void VeometriController::update(veometri::render::FPSCamera& camera, float dt, float mouseDx, float mouseDy)
{
    if (!m_window)
        return;

    float speed = m_orbitSpeed * dt;

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

    camera.rotate(mouseDx * m_mouseSensitivity, mouseDy * m_mouseSensitivity);
}


} // namespace veometri::app
