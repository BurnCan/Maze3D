#pragma once

#include <glm/glm.hpp>

namespace engine {

class Shader;
class CubeMesh;

class BoxRenderer {
public:
    explicit BoxRenderer(CubeMesh& cube);

    void draw(Shader& shader,
              const glm::vec3& position,
              const glm::vec3& scale);

private:
    CubeMesh& m_cube;
};

} // namespace engine
