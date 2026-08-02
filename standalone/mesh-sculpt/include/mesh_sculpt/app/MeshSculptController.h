#pragma once

#include <GLFW/glfw3.h>
#include "mesh_sculpt/app/ICameraController.h"



namespace mesh_sculpt::app
{

class MeshSculptController : public ICameraController
{
public:
    explicit MeshSculptController(GLFWwindow* window);

    void update(
        mesh_sculpt::render::FPSCamera& camera,
        float dt,
        float mouseDx,
        float mouseDy
    ) override;

private:
    GLFWwindow* m_window = nullptr;
    float m_orbitSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
};

} // namespace mesh_sculpt::app
