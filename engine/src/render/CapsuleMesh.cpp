#include "engine/render/CapsuleMesh.h"
#include <glm/glm.hpp>
#include <cmath>

using namespace engine;

CapsuleMesh::CapsuleMesh(float radius, float height, int segments, int rings)
{
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    float halfHeight = height * 0.5f - radius;

    // ----- Cylinder -----
    for (int i = 0; i <= segments; ++i)
    {
        float theta = (float)i / segments * 2.0f * 3.1415926f;
        float x = cos(theta) * radius;
        float z = sin(theta) * radius;

        vertices.emplace_back(x, -halfHeight, z);
        vertices.emplace_back(x,  halfHeight, z);
    }

    for (int i = 0; i < segments; ++i)
    {
        int start = i * 2;
        indices.push_back(start);
        indices.push_back(start + 1);
        indices.push_back(start + 2);

        indices.push_back(start + 1);
        indices.push_back(start + 3);
        indices.push_back(start + 2);
    }

    // ----- Hemispheres -----
    int baseIndex = vertices.size();

    for (int y = 0; y <= rings; ++y)
    {
        float v = (float)y / rings;
        float phi = v * 0.5f * 3.1415926f;

        for (int x = 0; x <= segments; ++x)
        {
            float u = (float)x / segments;
            float theta = u * 2.0f * 3.1415926f;

            float xr = cos(theta) * cos(phi);
            float yr = sin(phi);
            float zr = sin(theta) * cos(phi);

            // Top hemisphere
            vertices.emplace_back(
                xr * radius,
                yr * radius + halfHeight,
                zr * radius
            );

            // Bottom hemisphere
            vertices.emplace_back(
                xr * radius,
                -yr * radius - halfHeight,
                zr * radius
            );
        }
    }


    // Hemisphere Indexing
    int ringVerts = segments + 1;

    for (int y = 0; y < rings; ++y)
    {
        for (int x = 0; x < segments; ++x)
        {
            int i0 = baseIndex + (y * ringVerts + x) * 2;
            int i1 = i0 + ringVerts * 2;
            int i2 = i0 + 2;
            int i3 = i1 + 2;

            // Top hemisphere
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i3);

            // Bottom hemisphere (offset +1 because of interleaving)
            indices.push_back(i0 + 1);
            indices.push_back(i2 + 1);
            indices.push_back(i1 + 1);

            indices.push_back(i2 + 1);
            indices.push_back(i3 + 1);
            indices.push_back(i1 + 1);
        }
    }




    m_indexCount = indices.size();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(glm::vec3),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

CapsuleMesh::~CapsuleMesh()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
}

void CapsuleMesh::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
}
