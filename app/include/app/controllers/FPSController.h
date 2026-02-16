#pragma once

#include "ICameraController.h"
#include "engine/scene/FPSCamera.h"
#include <GLFW/glfw3.h>

namespace app
{

class FPSController : public ICameraController
{
public:
    explicit FPSController(GLFWwindow* window);

    void update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy) override;

    // Public getters for third-person distance
    float cameraDistance() const { return m_cameraDistance; }
    double scrollDelta() const { return m_scrollDelta; }

    // Return how much the player moved in world space this frame
    glm::vec3 movementDelta(const engine::FPSCamera& camera, float dt) const;




    void setCameraDistance(float distance)
    {
        m_cameraDistance = glm::clamp(distance, m_minDistance, m_maxDistance);
    }

    // Public scroll delta adder (used by GLFW callback)
    void addScrollDelta(double delta);



private:
    GLFWwindow* m_window = nullptr;



    float m_cameraDistance = 2.5f; // default distance behind player
    float m_minDistance    = 1.0f;
    float m_maxDistance    = 6.0f;
    double m_scrollDelta = 0.0;
};

} // namespace app
