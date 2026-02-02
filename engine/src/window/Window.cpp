#include "engine/window/Window.h"

#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// window resize callback
static void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

Window::Window(int width, int height, const char* title, bool fullscreen)
{
    if (!glfwInit())
        throw std::runtime_error("Failed to init GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* monitor = nullptr;
    int winW = width;
    int winH = height;

    if (fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        if (!monitor)
            throw std::runtime_error("Failed to get primary monitor");

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode)
            throw std::runtime_error("Failed to get video mode");

        winW = mode->width;
        winH = mode->height;
    }

    m_window = glfwCreateWindow(
        winW,
        winH,
        title,
        monitor,   // null = windowed, monitor = fullscreen
        nullptr
    );

    if (!m_window)
        throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGL())
        throw std::runtime_error("Failed to load OpenGL");

    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

    int widthPx, heightPx;
    glfwGetFramebufferSize(m_window, &widthPx, &heightPx);
    glViewport(0, 0, widthPx, heightPx);

    glfwSwapInterval(1);
}


Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents()
{
    glfwPollEvents();
}

void Window::swapBuffers()
{
    glfwSwapBuffers(m_window);
}
