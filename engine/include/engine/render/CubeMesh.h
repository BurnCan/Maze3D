#pragma once

#include <glad/glad.h>

namespace engine {

class CubeMesh {
public:
    CubeMesh();
    ~CubeMesh();

    CubeMesh(const CubeMesh&) = delete;
    CubeMesh& operator=(const CubeMesh&) = delete;

    void bind() const;
    void draw() const;

private:
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
};

}
