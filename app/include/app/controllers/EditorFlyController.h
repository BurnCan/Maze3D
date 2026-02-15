#pragma once

#include "engine/scene/FPSCamera.h"
#include "app/controllers/ICameraController.h"
#include <GLFW/glfw3.h>

namespace app
{

class EditorFlyController : public ICameraController
{
public:
    explicit EditorFlyController(GLFWwindow* window);

    void update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy) override;

private:
    GLFWwindow* m_window = nullptr;
    float m_baseSpeed = 5.0f;
};

} // namespace app
