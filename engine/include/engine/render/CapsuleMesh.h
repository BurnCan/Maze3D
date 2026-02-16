#pragma once
#include <vector>
#include <glad/glad.h>

namespace engine
{
class CapsuleMesh
{
public:
    CapsuleMesh(float radius, float height, int segments = 16, int rings = 8);
    ~CapsuleMesh();

    void draw() const;

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLsizei m_indexCount = 0;
};
}
