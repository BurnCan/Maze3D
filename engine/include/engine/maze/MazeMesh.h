#pragma once

#include <glad/glad.h>

namespace engine {

class Maze;
class Shader;

class MazeMesh {
public:
    MazeMesh();
    ~MazeMesh();

    void build(const Maze& maze);
    void draw(Shader& shader) const;

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    GLsizei m_vertexCount = 0;
};

} // namespace engine
