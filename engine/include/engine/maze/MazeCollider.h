#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace engine {

class Maze;

class MazeCollider {
public:
    struct AABB {
        glm::vec3 min;
        glm::vec3 max;
    };

public:
    void build(const Maze& maze);

    // resolves collision for a sphere
    void resolve(
        glm::vec3& position,
        float radius
    ) const;

private:
    bool sphereIntersectsAABB(
        const glm::vec3& center,
        float radius,
        const AABB& box
    ) const;

private:
    std::vector<AABB> m_walls;
};

} // namespace engine
