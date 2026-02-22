#pragma once

struct GLFWwindow;

namespace engine {

class Window {
public:
    Window(
        int width,
        int height,
        const char* title,
        bool fullscreen = true   // default to fullscreen
    );

    ~Window();

    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    GLFWwindow* nativeHandle() const { return m_window; }

private:
    GLFWwindow* m_window{};
};

} // namespace engine
