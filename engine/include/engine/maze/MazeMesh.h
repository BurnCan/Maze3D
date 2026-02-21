#pragma once

#include <glad/glad.h>
#include <vector>
#include <map>
#include <utility>
#include <glm/glm.hpp>

#include "engine/maze/MazeTypes.h"

namespace engine {

class Maze;
class Shader;

struct WallEdit {
    int x;
    int y;
    Direction dir;
    bool add;
};

struct CellRange {
    size_t offset;  // float offset in m_vertices
    size_t count;   // float count
};

class MazeMesh {
public:
    MazeMesh();
    ~MazeMesh();

    void build(const Maze& maze);
    void draw(Shader& shader) const;

    void editWall(const Maze& maze, const WallEdit& edit);
    void editCell(int x, int y, const Maze& maze);

private:
    void rebuildCell(int x, int y, const Maze& maze);

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    GLsizei m_vertexCount = 0;

    std::vector<float> m_vertices;

    std::map<std::pair<int,int>, CellRange> m_cellRanges;
};

} // namespace engine
