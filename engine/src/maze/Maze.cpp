#include "engine/maze/Maze.h"

#include <random>
#include <algorithm>

#include <stack>
#include "engine/maze/MazeTypes.h"

namespace engine {

Maze::Maze(int width, int height)
    : m_width(width), m_height(height),
      m_cells(width * height)
{
    for (auto& c : m_cells) {
        c.walls = North | East | South | West;
        c.visited = false;
    }
}

bool Maze::inBounds(int x, int y) const
{
    return x >= 0 && y >= 0 && x < m_width && y < m_height;
}

int Maze::index(int x, int y) const
{
    return y * m_width + x;
}

const Maze::Cell& Maze::cell(int x, int y) const
{
    return m_cells[index(x, y)];
}

void Maze::generate()
{
    std::mt19937 rng(std::random_device{}());

    for (auto& c : m_cells) {
        c.visited = false;
        c.walls = North | East | South | West;
    }

    carve(0, 0, rng);
}



//Recursive Bactracker
void Maze::carve(int x, int y, std::mt19937& rng)

{
    m_cells[index(x, y)].visited = true;

    static std::random_device rd;


    struct Step {
        int dx, dy;
        Direction dir;
        Direction opposite;
    };

    std::vector<Step> dirs = {
        { 0, -1, North, South },
        { 1,  0, East,  West  },
        { 0,  1, South, North },
        { -1, 0, West,  East  }
    };

    std::shuffle(dirs.begin(), dirs.end(), rng);

    for (const auto& s : dirs) {
        int nx = x + s.dx;
        int ny = y + s.dy;

        if (!inBounds(nx, ny))
            continue;

        auto& next = m_cells[index(nx, ny)];
        if (next.visited)
            continue;

        // remove walls between cells
        m_cells[index(x, y)].walls &= ~s.dir;
        next.walls &= ~s.opposite;

        carve(nx, ny, rng);

    }
}

} // namespace engine
