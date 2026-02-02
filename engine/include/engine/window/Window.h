#pragma once

struct GLFWwindow;

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

private:
    GLFWwindow* m_window{};
};
