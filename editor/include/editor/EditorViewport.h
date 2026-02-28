#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <app/controllers/ICameraController.h>


struct GLFWwindow;
namespace engine {
    class FPSCamera;   // ✅ correct namespace
}


class EditorViewport
{
public:
    explicit EditorViewport(GLFWwindow* window, const char* windowName = "Game");
    ~EditorViewport();





    void begin(engine::FPSCamera& camera);  // now matches global type

     //  Getter for controller
    app::ICameraController* getController() const {
        return m_controller.get();
    }

    // Public setter for the controller
     void setController(std::unique_ptr<app::ICameraController> controller)
    {
        m_controller = std::move(controller);
    }

    void end();
    void bindFramebuffer();

    glm::ivec2 size() const { return m_size; }
    glm::vec2 imageMin() const { return m_imageMin; }
    glm::vec2 imageMax() const { return m_imageMax; }

private:
    void resize(int w, int h);

    GLFWwindow* m_window{};
    unsigned int m_fbo{};
    unsigned int m_color{};
    unsigned int m_depth{};
    glm::ivec2 m_size{1280, 720};
    bool m_capturingMouse = false;
    std::unique_ptr<app::ICameraController> m_controller;
    const char* m_windowName = "Game";
    glm::vec2 m_imageMin{0.0f, 0.0f};
    glm::vec2 m_imageMax{0.0f, 0.0f};
};
