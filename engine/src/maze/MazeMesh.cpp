#include "engine/maze/MazeMesh.h"
#include "engine/maze/Maze.h"
#include "engine/maze/MazeTypes.h"
#include "engine/render/Shader.h"

#include <vector>
#include <glm/glm.hpp>

namespace engine {

MazeMesh::MazeMesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
}

MazeMesh::~MazeMesh()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void MazeMesh::build(const Maze& maze)
{
    constexpr float CELL_SIZE      = 1.0f;
    constexpr float WALL_HEIGHT    = 1.0f;
    constexpr float WALL_THICKNESS = 0.1f;

    std::vector<float> vertices;

    const float h = WALL_HEIGHT * 0.5f;
    const float t = WALL_THICKNESS * 0.5f;

    auto pushBox = [&](const glm::vec3& center, const glm::vec3& half)
    {
        glm::vec3 p[8] = {
            center + glm::vec3(-half.x, -half.y, -half.z),
            center + glm::vec3( half.x, -half.y, -half.z),
            center + glm::vec3( half.x,  half.y, -half.z),
            center + glm::vec3(-half.x,  half.y, -half.z),

            center + glm::vec3(-half.x, -half.y,  half.z),
            center + glm::vec3( half.x, -half.y,  half.z),
            center + glm::vec3( half.x,  half.y,  half.z),
            center + glm::vec3(-half.x,  half.y,  half.z),
        };

        auto tri = [&](int a, int b, int c)
        {
            vertices.insert(vertices.end(), {
                p[a].x, p[a].y, p[a].z,
                p[b].x, p[b].y, p[b].z,
                p[c].x, p[c].y, p[c].z
            });
        };

        //tri(0,1,2); tri(0,2,3); // back
        //tri(5,4,7); tri(5,7,6); // front
        //tri(4,0,3); tri(4,3,7); // left
        //tri(1,5,6); tri(1,6,2); // right
        //tri(3,2,6); tri(3,6,7); // top
        //tri(4,5,1); tri(4,1,0); // bottom

        // -Z (back)
        tri(0, 2, 1);
        tri(0, 3, 2);

        // +Z (front)
        tri(4, 5, 6);
        tri(4, 6, 7);

        // -X (left)
        tri(0, 4, 7);
        tri(0, 7, 3);

        // +X (right)
        tri(1, 2, 6);
        tri(1, 6, 5);

        // +Y (top)
        tri(3, 7, 6);
        tri(3, 6, 2);

        // -Y (bottom)
        tri(0, 1, 5);
        tri(0, 5, 4);


    };

    for (int y = 0; y < maze.height(); ++y) {
        for (int x = 0; x < maze.width(); ++x) {
            const auto& cell = maze.cell(x, y);

            float fx = x * CELL_SIZE;
            float fz = y * CELL_SIZE;

            // north
            if (cell.walls & North)
                pushBox({ fx + CELL_SIZE * 0.5f, h, fz },
                        { CELL_SIZE * 0.5f, h, t });

            // west
            if (cell.walls & West)
                pushBox({ fx, h, fz + CELL_SIZE * 0.5f },
                        { t, h, CELL_SIZE * 0.5f });

            // south border
            if ((cell.walls & South) && y == maze.height() - 1)
                pushBox({ fx + CELL_SIZE * 0.5f, h, fz + CELL_SIZE },
                        { CELL_SIZE * 0.5f, h, t });

            // east border
            if ((cell.walls & East) && x == maze.width() - 1)
                pushBox({ fx + CELL_SIZE, h, fz + CELL_SIZE * 0.5f },
                        { t, h, CELL_SIZE * 0.5f });
        }
    }

    m_vertexCount = static_cast<GLsizei>(vertices.size() / 3);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(float),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void MazeMesh::draw(Shader& shader) const
{
    if (m_vertexCount == 0)
        return;

    shader.setMat4("uModel", glm::mat4(1.0f));

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}

} // namespace engine
