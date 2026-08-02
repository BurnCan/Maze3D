#pragma once
#include <string_view>

struct GLFWwindow;

namespace mesh_sculpt::platform {

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
    void setTitle(std::string_view title);
    GLFWwindow* nativeHandle() const { return m_window; }

private:
    GLFWwindow* m_window{};
};

} // namespace mesh_sculpt::platform
