#pragma once
#include "engine/scene/FPSCamera.h"
#include <GLFW/glfw3.h>

namespace tools::mesh_sculpt
{

class MeshSculptController
{
public:
    explicit MeshSculptController(GLFWwindow* window);

    // Updated to include cameraControl toggle
    void update(engine::FPSCamera& camera, float dt, float mouseDx, float mouseDy, bool cameraControl);

private:
    GLFWwindow* m_window = nullptr;
    float m_orbitSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
};

} // namespace tools::mesh_sculpt
