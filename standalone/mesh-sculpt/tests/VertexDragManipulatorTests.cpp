#include "mesh_sculpt/sculpt/VertexDragManipulator.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

using mesh_sculpt::sculpt::Ray;
using mesh_sculpt::sculpt::VertexDragManipulator;

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool near(float a, float b, float epsilon = 1.0e-4f) { return std::abs(a - b) < epsilon; }
bool near(const glm::vec3& a, const glm::vec3& b)
{
    return near(a.x, b.x) && near(a.y, b.y) && near(a.z, b.z);
}
bool finite(const glm::vec3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}
}

int main()
{
    const Ray initial{{0, 0, 0}, {0, 0, -1}};
    VertexDragManipulator drag;
    check(!drag.isActive(), "default manipulator is inactive");
    check(!drag.update(initial), "inactive update has no result");
    check(drag.begin({0, 0, -2}, {0, 0, -1}, initial), "valid begin succeeds");
    check(drag.isActive(), "valid begin activates drag");
    drag.end();
    check(!drag.isActive(), "end deactivates drag");
    drag.end();
    check(!drag.isActive(), "repeated end is safe");

    check(!drag.begin({0, 0, -2}, {0, 0, 0}, initial), "zero normal is rejected");
    check(!drag.isActive(), "rejected begin remains inactive");

    check(drag.begin({0, 0, -2}, {0, 0, 1}, initial), "drag fixture begins");
    check(!drag.update({{0, 0, 0}, {1, 0, 0}}), "parallel ray has no intersection");
    check(!drag.update({{0, 0, 0}, {1, 0, 1.0e-7f}}), "nearly parallel ray is safe");
    const auto expected = drag.update({{1, 2, 0}, {0, 0, -1}});
    check(expected && near(*expected, {1, 2, -2}), "ray intersects at expected position");
    check(expected && finite(*expected), "returned position is finite");
    check(!drag.update({{0, 0, -3}, {0, 0, -1}}), "intersection behind origin is rejected");
    const auto repeated = drag.update({{1, 2, 0}, {0, 0, -1}});
    check(repeated && expected && near(*repeated, *expected), "repeated update preserves state");

    check(drag.begin({0, 0, -4}, {0, 0, 1}, initial), "begin while active restarts drag");
    const auto restarted = drag.update(initial);
    check(restarted && near(*restarted, {0, 0, -4}), "restart replaces previous plane");
    check(!drag.begin({0, 0, -2}, {0, 0, 0}, initial) && !drag.isActive(),
          "failed restart deterministically ends previous drag");

    VertexDragManipulator unitNormal;
    VertexDragManipulator scaledNormal;
    check(unitNormal.begin({0, 0, -3}, {0, 0, 1}, initial) &&
          scaledNormal.begin({0, 0, -3}, {0, 0, 25}, initial),
          "normalization fixtures begin");
    const Ray offsetRay{{2, -1, 0}, {0, 0, -2}};
    const auto unitResult = unitNormal.update(offsetRay);
    const auto scaledResult = scaledNormal.update(offsetRay);
    check(unitResult && scaledResult && near(*unitResult, *scaledResult),
          "normal magnitude does not change result");

    VertexDragManipulator noOffset;
    const glm::vec3 offRayVertex{0.08f, 0.0f, -2.0f};
    check(noOffset.begin(offRayVertex, initial.direction, initial), "off-ray vertex drag begins");
    const auto snapped = noOffset.update(initial);
    check(snapped && near(*snapped, {0, 0, -2}),
          "historical behavior applies no grab offset");

    VertexDragManipulator cameraFacing;
    const Ray cameraRay{{1, 2, 3}, {0, 0, -2}};
    check(cameraFacing.begin({1.05f, 2, -5}, cameraRay.direction, cameraRay),
          "representative camera-facing drag begins");
    const auto cameraResult = cameraFacing.update({{1, 2, 3}, {0.25f, -0.5f, -2}});
    // Old equation: t = -(dot(n, origin) + d) / dot(n, direction), d = -dot(n, vertex).
    check(cameraResult && near(*cameraResult, {2, 0, -5}),
          "camera-facing drag matches old plane calculation");

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
