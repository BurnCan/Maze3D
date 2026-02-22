#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace engine {

class DynamicMesh
{
public:
    DynamicMesh();
    ~DynamicMesh();

    void setVertices(const std::vector<glm::vec3>& verts);
    void setIndices(const std::vector<unsigned int>& indices);

    std::vector<glm::vec3>& vertices();
    const std::vector<glm::vec3>& vertices() const;

    std::vector<unsigned int>& indices();
    const std::vector<unsigned int>& indices() const;

    void upload();
    void draw() const;

    GLuint vao() const { return m_vao; }

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    std::vector<glm::vec3> m_vertices;
    std::vector<unsigned int> m_indices;

    bool m_hasIndices = false;
};

} // namespace engine
