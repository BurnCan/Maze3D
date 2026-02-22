#include "engine/render/DynamicMesh.h"

namespace engine {

DynamicMesh::DynamicMesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
}

DynamicMesh::~DynamicMesh()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}

void DynamicMesh::setVertices(const std::vector<glm::vec3>& verts)
{
    m_vertices = verts;
}

void DynamicMesh::setIndices(const std::vector<unsigned int>& indices)
{
    m_indices = indices;
    m_hasIndices = !m_indices.empty();
}

std::vector<glm::vec3>& DynamicMesh::vertices()
{
    return m_vertices;
}

const std::vector<glm::vec3>& DynamicMesh::vertices() const
{
    return m_vertices;
}

std::vector<unsigned int>& DynamicMesh::indices()
{
    return m_indices;
}

const std::vector<unsigned int>& DynamicMesh::indices() const
{
    return m_indices;
}

void DynamicMesh::upload()
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m_vertices.size() * sizeof(glm::vec3),
                 m_vertices.data(),
                 GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3),
                          (void*)0);
    glEnableVertexAttribArray(0);

    if (m_hasIndices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_indices.size() * sizeof(unsigned int),
                     m_indices.data(),
                     GL_DYNAMIC_DRAW);
    }

    glBindVertexArray(0);
}

void DynamicMesh::draw() const
{
    glBindVertexArray(m_vao);

    if (m_hasIndices)
    {
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(m_indices.size()),
                       GL_UNSIGNED_INT,
                       nullptr);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES,
                     0,
                     static_cast<GLsizei>(m_vertices.size()));
    }

    glBindVertexArray(0);
}

} // namespace engine
