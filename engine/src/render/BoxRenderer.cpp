#include "engine/render/BoxRenderer.h"
#include "engine/render/Shader.h"
#include "engine/render/CubeMesh.h"


#include <glm/gtc/matrix_transform.hpp>

namespace engine {

BoxRenderer::BoxRenderer(CubeMesh& cube)
    : m_cube(cube)
{
}

void BoxRenderer::draw(Shader& shader,
                       const glm::vec3& position,
                       const glm::vec3& scale)
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);

    shader.setMat4("uModel", model);
    m_cube.draw();
}

} // namespace engine
