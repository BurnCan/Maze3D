#include "engine/maze/MazeMesh.h"
#include "engine/maze/Maze.h"
#include "engine/maze/MazeTypes.h"
#include "engine/render/Shader.h"

#include <vector>
#include <utility>
#include <functional> // std::hash
#include <unordered_map>
#include <algorithm>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>



namespace engine {



// -------------------- Constructor / Destructor --------------------
MazeMesh::MazeMesh() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
}

MazeMesh::~MazeMesh() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

// -------------------- Full Maze Build --------------------
void MazeMesh::build(const Maze& maze)
{
    m_vertices.clear();
    m_cellRanges.clear();

    for (int y = 0; y < maze.height(); ++y)
    {
        for (int x = 0; x < maze.width(); ++x)
        {
            rebuildCell(x, y, maze);
        }
    }

    m_vertexCount = static_cast<GLsizei>(m_vertices.size() / 3);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m_vertices.size() * sizeof(float),
                 m_vertices.data(),
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    std::cout << "Vertex count: " << m_vertexCount << std::endl;
}


// -------------------- Draw --------------------
void MazeMesh::draw(Shader& shader) const {
    if (m_vertexCount == 0) return;

    shader.setMat4("uModel", glm::mat4(1.0f));
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);
}





// -------------------- Rebuild Single Cell --------------------
void MazeMesh::rebuildCell(int x, int y, const Maze& maze)
{
    constexpr float CELL_SIZE      = 1.0f;
    constexpr float WALL_HEIGHT    = 1.0f;
    constexpr float WALL_THICKNESS = 0.1f;
    float h = WALL_HEIGHT * 0.5f;
    float t = WALL_THICKNESS * 0.5f;

    const auto& cell = maze.cell(x, y);

    float fx = x * CELL_SIZE;
    float fz = y * CELL_SIZE;

    std::vector<float> newVerts;

    auto pushBox = [&](glm::vec3 center, glm::vec3 half)
    {
        glm::vec3 p[8] = {
            center + glm::vec3(-half.x,-half.y,-half.z),
            center + glm::vec3( half.x,-half.y,-half.z),
            center + glm::vec3( half.x, half.y,-half.z),
            center + glm::vec3(-half.x, half.y,-half.z),
            center + glm::vec3(-half.x,-half.y, half.z),
            center + glm::vec3( half.x,-half.y, half.z),
            center + glm::vec3( half.x, half.y, half.z),
            center + glm::vec3(-half.x, half.y, half.z)
        };

        auto tri = [&](int a,int b,int c){
            newVerts.insert(newVerts.end(), {
                p[a].x,p[a].y,p[a].z,
                p[b].x,p[b].y,p[b].z,
                p[c].x,p[c].y,p[c].z
            });
        };

        tri(0,2,1); tri(0,3,2);
        tri(4,5,6); tri(4,6,7);
        tri(0,4,7); tri(0,7,3);
        tri(1,2,6); tri(1,6,5);
        tri(3,7,6); tri(3,6,2);
        tri(0,1,5); tri(0,5,4);
    };

    if (cell.walls & North)
        pushBox({ fx + CELL_SIZE*0.5f, h, fz }, { CELL_SIZE*0.5f, h, t });

    if (cell.walls & West)
        pushBox({ fx, h, fz + CELL_SIZE*0.5f }, { t, h, CELL_SIZE*0.5f });

    if ((cell.walls & South) && y == maze.height()-1)
        pushBox({ fx + CELL_SIZE*0.5f, h, fz + CELL_SIZE }, { CELL_SIZE*0.5f, h, t });

    if ((cell.walls & East) && x == maze.width()-1)
        pushBox({ fx + CELL_SIZE, h, fz + CELL_SIZE*0.5f }, { t, h, CELL_SIZE*0.5f });

    auto key = std::make_pair(x,y);

    // Remove old data
    if (m_cellRanges.count(key))
    {
        auto old = m_cellRanges[key];

        m_vertices.erase(
            m_vertices.begin() + old.offset,
            m_vertices.begin() + old.offset + old.count
        );

        // Fix offsets of all cells after this one
        for (auto& [k, range] : m_cellRanges)
        {
            if (range.offset > old.offset)
                range.offset -= old.count;
        }
    }

    // Insert new data at end
    size_t newOffset = m_vertices.size();
    m_vertices.insert(m_vertices.end(), newVerts.begin(), newVerts.end());

    m_cellRanges[key] = { newOffset, newVerts.size() };

    m_vertexCount = static_cast<GLsizei>(m_vertices.size() / 3);
}


// -------------------- Edit Single Wall --------------------
void MazeMesh::editWall(const Maze& maze, const WallEdit& edit)
{
    // Rebuild edited cell
    rebuildCell(edit.x, edit.y, maze);

    // Rebuild neighbor if wall is shared
    switch (edit.dir)
    {
        case North:
            if (edit.y > 0)
                rebuildCell(edit.x, edit.y - 1, maze);
            break;

        case West:
            if (edit.x > 0)
                rebuildCell(edit.x - 1, edit.y, maze);
            break;

        case South:
            if (edit.y < maze.height() - 1)
                rebuildCell(edit.x, edit.y + 1, maze);
            break;

        case East:
            if (edit.x < maze.width() - 1)
                rebuildCell(edit.x + 1, edit.y, maze);
            break;
    }

    // Update GPU once after all changes
    m_vertexCount = static_cast<GLsizei>(m_vertices.size() / 3);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m_vertices.size() * sizeof(float),
                 m_vertices.data(),
                 GL_DYNAMIC_DRAW);
}

void engine::MazeMesh::editCell(int x, int y, const Maze& maze) {
    rebuildCell(x, y, maze);
}

} // namespace engine
