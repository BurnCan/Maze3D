#pragma once

struct GLFWwindow;

namespace engine {

class FPSCamera;

class FPSController
{
public:
    FPSController(GLFWwindow* window);

    void update(FPSCamera& camera, float dt, float mouseDx, float mouseDy);

    void setMouseCaptured(bool captured);

private:
    GLFWwindow* m_window{};
    float m_speed = 5.0f;
};

}
