#pragma once

#include <GLFW/glfw3.h>
#include "veometri/app/ICameraController.h"



namespace veometri::app
{

class VeometriController : public ICameraController
{
public:
    explicit VeometriController(GLFWwindow* window);

    void update(
        veometri::render::FPSCamera& camera,
        float dt,
        float mouseDx,
        float mouseDy
    ) override;

private:
    GLFWwindow* m_window = nullptr;
    float m_orbitSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
};

} // namespace veometri::app
