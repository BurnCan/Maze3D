#include "veometri/sculpt/VeometriRenderer.h"

#include "veometri/render/Camera.h"
#include "veometri/render/DynamicMesh.h"
#include "veometri/render/Shader.h"
#include "veometri/platform/AssetLocator.h"
#include "veometri/sculpt/VeometriRenderData.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

namespace veometri::sculpt {
namespace {

std::unique_ptr<render::Shader> loadShader(
    const char* role, const std::filesystem::path& vertex, const std::filesystem::path& fragment)
{
    try
    {
        return std::make_unique<render::Shader>(vertex, fragment);
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error(
            std::string("Failed to load ") + role + " shader (vertex: " + vertex.string() +
            ", fragment: " + fragment.string() + "): " + error.what());
    }
}

class ScopedRenderState
{
public:
    ScopedRenderState()
    {
        glGetIntegerv(GL_CURRENT_PROGRAM, &m_program);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_vao);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_arrayBuffer);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_elementBuffer);
        glGetIntegerv(GL_POLYGON_MODE, m_polygonMode);
        glGetFloatv(GL_POINT_SIZE, &m_pointSize);
        m_programPointSize = glIsEnabled(GL_PROGRAM_POINT_SIZE);
        m_polygonOffsetFill = glIsEnabled(GL_POLYGON_OFFSET_FILL);
    }

    ~ScopedRenderState()
    {
        glPolygonMode(GL_FRONT, m_polygonMode[0]);
        glPolygonMode(GL_BACK, m_polygonMode[1]);
        glPointSize(m_pointSize);
        setEnabled(GL_PROGRAM_POINT_SIZE, m_programPointSize);
        setEnabled(GL_POLYGON_OFFSET_FILL, m_polygonOffsetFill);
        glUseProgram(static_cast<GLuint>(m_program));
        glBindVertexArray(static_cast<GLuint>(m_vao));
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(m_arrayBuffer));
        // In a core profile the element binding belongs to a VAO, and binding
        // one while VAO zero is active is invalid.
        if (m_vao != 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(m_elementBuffer));
    }

    ScopedRenderState(const ScopedRenderState&) = delete;
    ScopedRenderState& operator=(const ScopedRenderState&) = delete;

private:
    static void setEnabled(GLenum capability, GLboolean enabled)
    {
        enabled == GL_TRUE ? glEnable(capability) : glDisable(capability);
    }

    GLint m_program = 0;
    GLint m_vao = 0;
    GLint m_arrayBuffer = 0;
    GLint m_elementBuffer = 0;
    GLint m_polygonMode[2] = {GL_FILL, GL_FILL};
    GLfloat m_pointSize = 1.0f;
    GLboolean m_programPointSize = GL_FALSE;
    GLboolean m_polygonOffsetFill = GL_FALSE;
};

} // namespace

class VeometriRenderer::Impl
{
public:
    explicit Impl(const render::Camera* camera) : camera(camera)
    {
        const auto basicVertex = platform::AssetLocator::resolve("shaders/basic.vert");
        const auto basicFragment = platform::AssetLocator::resolve("shaders/basic.frag");
        shader = loadShader("base", basicVertex, basicFragment);
        highlightShader = loadShader(
            "highlight", platform::AssetLocator::resolve("shaders/highlight.vert"),
            platform::AssetLocator::resolve("shaders/highlight.frag"));
    }

    const render::Camera* camera;
    render::DynamicMesh mesh;
    std::unique_ptr<render::Shader> shader;
    std::unique_ptr<render::Shader> highlightShader;
};

VeometriRenderer::VeometriRenderer(const render::Camera* camera)
    : m_impl(std::make_unique<Impl>(camera))
{
}

VeometriRenderer::~VeometriRenderer() = default;

void VeometriRenderer::upload(const SculptMesh& mesh)
{
    m_impl->mesh.setVertices(mesh.vertices());
    m_impl->mesh.setIndices(mesh.indices());
    m_impl->mesh.upload();
}

void VeometriRenderer::render(const SculptMesh& mesh, const MeshSelection& selection)
{
    if (!m_impl->camera)
        return;

    ScopedRenderState stateGuard;
    const glm::mat4 model(1.0f);
    const glm::mat4& view = m_impl->camera->view();
    const glm::mat4& projection = m_impl->camera->projection();

    m_impl->shader->bind();
    m_impl->shader->setMat4("uModel", model);
    m_impl->shader->setMat4("uView", view);
    m_impl->shader->setMat4("uProj", projection);
    m_impl->shader->setVec3("uColor", glm::vec3(0.7f, 0.7f, 0.8f));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_impl->mesh.draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(m_impl->mesh.vao());
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(8.0f);
    m_impl->shader->setVec3("uColor", glm::vec3(0.2f, 0.9f, 0.3f));
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(mesh.vertices().size()));

    if (const auto vertex = selection.selectedVertex();
        vertex && selectedVertexPosition(mesh, *vertex))
    {
        m_impl->highlightShader->bind();
        m_impl->highlightShader->setMat4("uModel", model);
        m_impl->highlightShader->setMat4("uView", view);
        m_impl->highlightShader->setMat4("uProj", projection);
        m_impl->highlightShader->setVec3("uColor", glm::vec3(1.0f, 0.2f, 0.2f));
        glPointSize(18.0f);
        glDrawArrays(GL_POINTS, static_cast<GLint>(*vertex), 1);
    }

    if (const auto triangle = selection.selectedTriangle();
        triangle && selectedTrianglePositions(mesh, *triangle))
    {
        const std::size_t triangleBase = *triangle * 3;
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_impl->highlightShader->bind();
        m_impl->highlightShader->setMat4("uModel", model);
        m_impl->highlightShader->setMat4("uView", view);
        m_impl->highlightShader->setMat4("uProj", projection);
        m_impl->highlightShader->setVec3("uColor", glm::vec3(1.0f, 0.3f, 0.1f));
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT,
            reinterpret_cast<void*>(triangleBase * sizeof(unsigned int)));
    }
}

} // namespace veometri::sculpt
