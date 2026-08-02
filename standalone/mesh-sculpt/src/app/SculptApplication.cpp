#include "mesh_sculpt/app/SculptApplication.h"

#include "mesh_sculpt/app/MeshSculptController.h"
#include "mesh_sculpt/platform/Window.h"
#include "mesh_sculpt/render/FPSCamera.h"
#include "mesh_sculpt/sculpt/MeshSculptTool.h"
#include "mesh_sculpt/sculpt/MeshSculptUi.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <portable-file-dialogs.h>

namespace mesh_sculpt::app
{

class SculptApplication::Impl
{
public:
    Impl()
        : m_window(std::make_unique<platform::Window>(1280, 720, "Mesh Sculpt Tool"))
    {
        try
        {
            initializeGraphics();
            initializeImGui();
            initializeApplicationObjects();
        }
        catch (...)
        {
            shutdownImGui();
            throw;
        }
    }

    ~Impl() noexcept
    {
        shutdownImGui();

        // Release resources which may issue OpenGL calls before destroying the window.
        m_controller.reset();
        m_ui.reset();
        m_tool.reset();
        m_camera.reset();
    }

    int run()
    {
        while (!m_window->shouldClose())
        {
            processFrameTiming();
            m_window->pollEvents();
            processModeToggle();
            processMouseDelta();
            processInput();
            update();
            renderScene();
            renderUi();
            endFrame();
        }

        return EXIT_SUCCESS;
    }

private:
    void initializeGraphics()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void initializeImGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        if (ImGui::GetCurrentContext() == nullptr)
            throw std::runtime_error("Failed to create ImGui context");
        m_imguiContextCreated = true;

        ImGui::StyleColorsDark();
        ImGui::GetIO().FontGlobalScale = 3.2f;

        if (!ImGui_ImplGlfw_InitForOpenGL(m_window->nativeHandle(), true))
            throw std::runtime_error("Failed to initialize ImGui GLFW backend");
        m_imguiGlfwInitialized = true;

        if (!ImGui_ImplOpenGL3_Init("#version 450"))
            throw std::runtime_error("Failed to initialize ImGui OpenGL backend");
        m_imguiOpenGlInitialized = true;
    }

    void initializeApplicationObjects()
    {
        m_camera = std::make_unique<render::FPSCamera>(
            45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
        m_tool = std::make_unique<sculpt::MeshSculptTool>(m_camera.get());
        m_ui = std::make_unique<sculpt::MeshSculptUi>();
        m_controller = std::make_unique<MeshSculptController>(m_window->nativeHandle());

        m_lastTime = static_cast<float>(glfwGetTime());
        glfwGetCursorPos(m_window->nativeHandle(), &m_lastMouseX, &m_lastMouseY);
        glfwSetInputMode(m_window->nativeHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void shutdownImGui() noexcept
    {
        if (m_imguiOpenGlInitialized)
        {
            ImGui_ImplOpenGL3_Shutdown();
            m_imguiOpenGlInitialized = false;
        }
        if (m_imguiGlfwInitialized)
        {
            ImGui_ImplGlfw_Shutdown();
            m_imguiGlfwInitialized = false;
        }
        if (m_imguiContextCreated)
        {
            ImGui::DestroyContext();
            m_imguiContextCreated = false;
        }
    }

    void processFrameTiming()
    {
        const float currentTime = static_cast<float>(glfwGetTime());
        m_deltaTime = currentTime - m_lastTime;
        m_lastTime = currentTime;
    }

    void processModeToggle()
    {
        const bool tabPressedNow =
            glfwGetKey(m_window->nativeHandle(), GLFW_KEY_TAB) == GLFW_PRESS;

        if (tabPressedNow && !m_tabPressedLastFrame)
        {
            m_cameraControl = !m_cameraControl;
            const int cursorMode = m_cameraControl ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
            glfwSetInputMode(m_window->nativeHandle(), GLFW_CURSOR, cursorMode);

            if (m_cameraControl)
                glfwGetCursorPos(m_window->nativeHandle(), &m_lastMouseX, &m_lastMouseY);
        }

        m_tabPressedLastFrame = tabPressedNow;
    }

    void processMouseDelta()
    {
        double mouseX = 0.0;
        double mouseY = 0.0;
        glfwGetCursorPos(m_window->nativeHandle(), &mouseX, &mouseY);

        m_mouseDx = static_cast<float>(mouseX - m_lastMouseX);
        m_mouseDy = static_cast<float>(mouseY - m_lastMouseY);
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;
    }

    void processInput()
    {
        if (m_cameraControl)
            m_controller->update(*m_camera, m_deltaTime, m_mouseDx, -m_mouseDy);

        m_leftClick = glfwGetMouseButton(
            m_window->nativeHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        m_deletePressed =
            glfwGetKey(m_window->nativeHandle(), GLFW_KEY_DELETE) == GLFW_PRESS;
    }

    void update()
    {
        m_tool->update(m_deltaTime, m_cameraControl, m_leftClick, m_deletePressed);
        std::string title = "Mesh Sculpt Tool — ";
        title += m_tool->currentDocumentPath() ?
            m_tool->currentDocumentPath()->filename().string() : "Untitled";
        if (m_tool->isDirty()) title += " *";
        m_window->setTitle(title);
    }

    void renderScene()
    {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_tool->render();
    }

    void renderUi()
    {
        ImGuiIO& io = ImGui::GetIO();
        if (m_cameraControl)
            io.ConfigFlags |= ImGuiConfigFlags_NoKeyboard;
        else
            io.ConfigFlags &= ~ImGuiConfigFlags_NoKeyboard;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processMenuAction(m_ui->renderMainMenuBar());
        m_tool->editorUi().renderOverlay(m_tool->camera(), m_tool->mesh(), m_tool->selection(),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(io.DisplaySize.x, io.DisplaySize.y),
            m_cameraControl);
        m_tool->processEditorAction(
            m_tool->editorUi().renderToolPanel(m_tool->mesh(), m_tool->selection()));
        m_ui->renderInfoPanel(m_cameraControl);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void processMenuAction(sculpt::MeshSculptUi::FileAction action)
    {
        switch (action)
        {
            case sculpt::MeshSculptUi::FileAction::NewDocument:
                m_tool->newDocument();
                break;
            case sculpt::MeshSculptUi::FileAction::OpenDocument:
            {
                const auto paths = pfd::open_file("Open geometry", "", {"Geometry files", "*.meshgeo"}).result();
                if (!paths.empty()) m_tool->openDocument(paths.front());
                break;
            }
            case sculpt::MeshSculptUi::FileAction::SaveDocument:
                if (m_tool->currentDocumentPath()) m_tool->saveDocument(*m_tool->currentDocumentPath());
                else saveAs();
                break;
            case sculpt::MeshSculptUi::FileAction::SaveDocumentAs:
                saveAs();
                break;
            case sculpt::MeshSculptUi::FileAction::ResetMesh:
                m_tool->resetMesh();
                break;
            case sculpt::MeshSculptUi::FileAction::Exit:
                requestClose();
                break;
            case sculpt::MeshSculptUi::FileAction::None:
                break;
        }
    }

    void saveAs()
    {
        auto selected = pfd::save_file("Save geometry", "untitled.meshgeo",
            {"Geometry files", "*.meshgeo"}).result();
        if (selected.empty()) return;
        std::filesystem::path path(selected);
        if (!path.has_extension()) path += ".meshgeo";
        m_tool->saveDocument(path);
    }

    void requestClose()
    {
        glfwSetWindowShouldClose(m_window->nativeHandle(), GLFW_TRUE);
    }

    void endFrame()
    {
        m_window->swapBuffers();
    }

    // Declared first so the OpenGL context outlives all graphics-dependent objects.
    std::unique_ptr<platform::Window> m_window;
    std::unique_ptr<render::FPSCamera> m_camera;
    std::unique_ptr<sculpt::MeshSculptTool> m_tool;
    std::unique_ptr<sculpt::MeshSculptUi> m_ui;
    std::unique_ptr<MeshSculptController> m_controller;

    float m_lastTime = 0.0f;
    float m_deltaTime = 0.0f;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    float m_mouseDx = 0.0f;
    float m_mouseDy = 0.0f;
    bool m_cameraControl = true;
    bool m_tabPressedLastFrame = false;
    bool m_leftClick = false;
    bool m_deletePressed = false;
    bool m_imguiContextCreated = false;
    bool m_imguiGlfwInitialized = false;
    bool m_imguiOpenGlInitialized = false;
};

SculptApplication::SculptApplication()
    : m_impl(std::make_unique<Impl>())
{
}

SculptApplication::~SculptApplication() = default;

int SculptApplication::run()
{
    return m_impl->run();
}

} // namespace mesh_sculpt::app
