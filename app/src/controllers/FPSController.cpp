#include "app/controllers/FPSController.h"
#include <glm/gtc/constants.hpp>

namespace app
{

// Scroll callback for GLFW
static void scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
    auto* controller = static_cast<FPSController*>(glfwGetWindowUserPointer(window));
    if (controller)
        controller->addScrollDelta(yoffset);
}

// ---------------------------
// Constructor
// ---------------------------
FPSController::FPSController(GLFWwindow* window)
    : m_window(window)
{
    if (window)
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetScrollCallback(window, scrollCallback);
    }
}

// ---------------------------
// This computes movement along the XZ plane relative to the camera’s yaw.
// ---------------------------
glm::vec3 FPSController::movementDelta(const engine::FPSCamera& camera, float dt) const
{
    float speed = 2.0f;
    glm::vec3 delta(0.0f);

    // Flatten forward vector to XZ plane
    glm::vec3 forward = camera.forward();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right = camera.right();
    right.y = 0.0f;
    if (glm::length(right) > 0.0f)
        right = glm::normalize(right);

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        delta += forward * speed * dt;
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        delta -= forward * speed * dt;
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        delta -= right * speed * dt;
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        delta += right * speed * dt;

    return delta;
}



// ---------------------------
// Add scroll delta (called by callback)
// ---------------------------
void FPSController::addScrollDelta(double delta)
{
    m_scrollDelta += delta;
}

// ---------------------------
// Update camera (called each frame)
// ---------------------------
void FPSController::update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy)
{
    if (!m_window) return;

    // ---------------------------
    // Movement
    // ---------------------------
    float baseSpeed = 5.0f;
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        baseSpeed *= 3.0f;

    float speed = baseSpeed * dt;

    if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS) camera.moveForward(speed);
    if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS) camera.moveForward(-speed);
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) camera.moveRight(-speed);
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) camera.moveRight(speed);
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.moveUp(speed);
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) camera.moveUp(-speed);

    // ---------------------------
    // Mouse look
    // ---------------------------
    camera.rotate(mouseDx, mouseDy);

    // ---------------------------
    // Scroll zoom (third-person distance)
    // ---------------------------
    if (m_scrollDelta != 0.0)
    {
        m_cameraDistance -= static_cast<float>(m_scrollDelta) * 0.5f; // sensitivity
        m_cameraDistance = glm::clamp(m_cameraDistance, m_minDistance, m_maxDistance);
        m_scrollDelta = 0.0;
    }
}

} // namespace app
