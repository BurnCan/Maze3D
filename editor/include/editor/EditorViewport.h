#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;
namespace engine {
    class FPSCamera;   // ✅ correct namespace
}


class EditorViewport
{
public:
    EditorViewport(GLFWwindow* window);
    ~EditorViewport();

    void begin(engine::FPSCamera& camera);  // now matches global type

    void end();
    void bindFramebuffer();

    glm::ivec2 size() const { return m_size; }

private:
    void resize(int w, int h);

    GLFWwindow* m_window{};
    unsigned int m_fbo{};
    unsigned int m_color{};
    unsigned int m_depth{};
    glm::ivec2 m_size{1280, 720};
    bool m_capturingMouse = false;
};
