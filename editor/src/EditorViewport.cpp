#include "editor/EditorViewport.h"

#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include "engine/scene/FPSCamera.h"

EditorViewport::EditorViewport(GLFWwindow* window)
    : m_window(window)
{
    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_color);
    glGenRenderbuffers(1, &m_depth);

    resize(m_size.x, m_size.y);
}

EditorViewport::~EditorViewport()
{
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_color);
    glDeleteRenderbuffers(1, &m_depth);
}

void EditorViewport::resize(int w, int h)
{
    if (w <= 0 || h <= 0) return;

    m_size = { w, h };

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // color
    glBindTexture(GL_TEXTURE_2D, m_color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color, 0);

    // depth
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Viewport framebuffer incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EditorViewport::begin(engine::FPSCamera& camera)
{
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin("Game");

    ImVec2 avail = ImGui::GetContentRegionAvail();

    if ((int)avail.x != m_size.x || (int)avail.y != m_size.y)
        resize((int)avail.x, (int)avail.y);

    ImGuiIO& io = ImGui::GetIO();

    // bigger editor text
    io.FontGlobalScale = 3.2f;

    // ---------------------------
    // mouse capture logic
    // ---------------------------
    bool hovered   = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
    bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (hovered && rightDown && !m_capturingMouse)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_capturingMouse = true;
    }

    if (m_capturingMouse && ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_capturingMouse = false;
    }

    // ---------------------------
    // Camera controller update
    // ---------------------------
    if (m_capturingMouse && m_controller)
    {
        ImGuiIO& io = ImGui::GetIO();
        float mouseDx = io.MouseDelta.x;
        float mouseDy = -io.MouseDelta.y;

        m_controller->update(camera, io.DeltaTime, mouseDx, mouseDy);
    }


    // ---------------------------
    // bind FBO for rendering
    // ---------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_size.x, m_size.y);

    camera.setAspectRatio((float)m_size.x / (float)m_size.y);
}

void EditorViewport::bindFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void EditorViewport::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ImGui::Image(
        (ImTextureID)(intptr_t)m_color,
        ImVec2((float)m_size.x, (float)m_size.y),
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    ImGui::End();
}

