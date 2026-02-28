#pragma once

#include <GLFW/glfw3.h>
#include "app/controllers/ICameraController.h"



namespace app
{

class MeshSculptController : public ICameraController
{
public:
    explicit MeshSculptController(GLFWwindow* window);

    void update(
        engine::FPSCamera& camera,
        float dt,
        float mouseDx,
        float mouseDy
    ) override;

private:
    GLFWwindow* m_window = nullptr;
    float m_orbitSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
};

} // namespace app
