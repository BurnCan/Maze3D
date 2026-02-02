#include "engine/maze/MazeCollider.h"
#include "engine/maze/Maze.h"
#include "engine/maze/MazeTypes.h"

#include <algorithm>

namespace engine {

void MazeCollider::build(const Maze& maze)
{
    constexpr float CELL = 1.0f;
    constexpr float WALL_HEIGHT = 1.0f;
    constexpr float WALL_THICKNESS = 0.1f;

    m_walls.clear();

    for (int y = 0; y < maze.height(); ++y) {
        for (int x = 0; x < maze.width(); ++x) {
            const auto& cell = maze.cell(x, y);

            float fx = x * CELL;
            float fz = y * CELL;

            auto addWall = [&](glm::vec3 min, glm::vec3 max) {
                m_walls.push_back({ min, max });
            };

            if (cell.walls & North)
                addWall(
                    { fx, 0, fz - WALL_THICKNESS },
                    { fx + CELL, WALL_HEIGHT, fz }
                );

            if (cell.walls & South)
                addWall(
                    { fx, 0, fz + CELL },
                    { fx + CELL, WALL_HEIGHT, fz + CELL + WALL_THICKNESS }
                );

            if (cell.walls & West)
                addWall(
                    { fx - WALL_THICKNESS, 0, fz },
                    { fx, WALL_HEIGHT, fz + CELL }
                );

            if (cell.walls & East)
                addWall(
                    { fx + CELL, 0, fz },
                    { fx + CELL + WALL_THICKNESS, WALL_HEIGHT, fz + CELL }
                );
        }
    }
}

bool MazeCollider::sphereIntersectsAABB(
    const glm::vec3& c,
    float r,
    const AABB& b
) const {
    float x = std::max(b.min.x, std::min(c.x, b.max.x));
    float y = std::max(b.min.y, std::min(c.y, b.max.y));
    float z = std::max(b.min.z, std::min(c.z, b.max.z));

    float dx = c.x - x;
    float dy = c.y - y;
    float dz = c.z - z;

    return (dx*dx + dy*dy + dz*dz) < (r * r);
}

// Resolve collisions (slide-friendly)
void MazeCollider::resolve(glm::vec3& pos, float radius) const
{
    for (const auto& wall : m_walls) {
        if (!sphereIntersectsAABB(pos, radius, wall))
            continue;

        // push out along smallest axis
        float left   = pos.x - wall.min.x;
        float right  = wall.max.x - pos.x;
        float front  = pos.z - wall.min.z;
        float back   = wall.max.z - pos.z;

        float minPen = std::min({ left, right, front, back });

        if (minPen == left)  pos.x = wall.min.x - radius;
        if (minPen == right) pos.x = wall.max.x + radius;
        if (minPen == front) pos.z = wall.min.z - radius;
        if (minPen == back)  pos.z = wall.max.z + radius;
    }
}

} // namespace engine

