#pragma once

#include <vector>
#include <cstdint>
#include <random>
#include "MazeTypes.h"

namespace engine {

class Maze {
public:
    struct Cell {
        uint8_t walls;   // bitmask of Direction
        bool visited;
    };

    Maze(int width, int height);

    void generate();

    // Const getter for read-only access
    const Cell& cell(int x, int y) const;

    // --- New: mutable helpers for editing walls ---
    void addWall(int x, int y, Direction dir);
    void removeWall(int x, int y, Direction dir);
    void clearWalls();

    int width() const  { return m_width; }
    int height() const { return m_height; }

private:
    bool inBounds(int x, int y) const;
    int index(int x, int y) const;
    void carve(int x, int y, std::mt19937& rng);

    int m_width;
    int m_height;
    std::vector<Cell> m_cells;
};

} // namespace engine
