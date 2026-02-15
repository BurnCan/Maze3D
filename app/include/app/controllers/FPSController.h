#pragma once

#include "engine/scene/FPSCamera.h"
#include "app/controllers/ICameraController.h"
#include <GLFW/glfw3.h>

namespace app
{

class FPSController : public ICameraController
{
public:
    // Constructor takes a GLFW window pointer for input polling
    explicit FPSController(GLFWwindow* window);

    // Override from ICameraController
    void update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy) override;

private:
    GLFWwindow* m_window = nullptr;  // Window reference for keyboard/mouse input
};

} // namespace app
