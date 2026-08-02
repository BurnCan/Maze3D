#include "veometri/sculpt/MeshPicker.h"
#include "veometri/sculpt/SculptMesh.h"
#include "veometri/render/Camera.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace veometri::sculpt;

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool near(float a, float b, float epsilon = 1.0e-4f) { return std::abs(a - b) < epsilon; }

SculptMesh meshFrom(const char* vertices, const char* indices = "")
{
    SculptMesh mesh;
    check(mesh.replaceFromText(vertices, indices).success, "mesh fixture parses");
    return mesh;
}

class TestCamera final : public veometri::render::Camera
{
public:
    TestCamera()
        : m_view(glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f))),
          m_projection(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f)) {}
    const glm::mat4& view() const override { return m_view; }
    const glm::mat4& projection() const override { return m_projection; }
    void setAspectRatio(float) override {}
    glm::vec3 position() const override { return {}; }
    glm::vec3 forward() const override { return {0.0f, 0.0f, -2.0f}; }
private:
    glm::mat4 m_view;
    glm::mat4 m_projection;
};
}

int main()
{
    const MeshPicker picker;
    const Ray forward{{0, 0, 0}, {0, 0, -1}};

    SculptMesh vertices = meshFrom("0.05 0 -2\n0.2 0 -1\n0 0 1\n0.05 0 -1");
    check(picker.pickVertex(vertices, forward) == 0, "nearby vertex selected by perpendicular ray distance");
    SculptMesh distant = meshFrom("0.2 0 -1");
    check(!picker.pickVertex(distant, forward), "distant vertex rejected");
    SculptMesh behind = meshFrom("0 0 1");
    check(!picker.pickVertex(behind, forward), "vertex behind origin rejected");
    SculptMesh closestRadius = meshFrom("0.08 0 -1\n0.02 0 -4");
    check(picker.pickVertex(closestRadius, forward) == 1, "smallest perpendicular distance wins");
    SculptMesh tie = meshFrom("0.05 0 -1\n-0.05 0 -2");
    check(picker.pickVertex(tie, forward) == 0, "vertex tie favors first ordinal");
    MeshPicker wide({0.25f, 0.0f, 1.0e-7f});
    check(wide.pickVertex(distant, forward) == 0, "configured vertex radius honored");

    SculptMesh triangle = meshFrom("-1 -1 -2\n1 -1 -2\n0 1 -2", "0 1 2");
    check(picker.pickTriangle(triangle, forward) == 0, "ray hits triangle");
    check(!picker.pickTriangle(triangle, {{2, 2, 0}, {0, 0, -1}}), "ray misses triangle");
    check(!picker.pickTriangle(triangle, {{0, 0, -3}, {0, 0, -1}}), "hit behind origin rejected");
    SculptMesh two = meshFrom("-1 -1 -4\n1 -1 -4\n0 1 -4\n-1 -1 -2\n1 -1 -2\n0 1 -2", "0 1 2\n3 4 5");
    check(picker.pickTriangle(two, forward) == 1, "nearest triangle selected and ordinal returned");
    SculptMesh ordinal = meshFrom("10 10 -1\n11 10 -1\n10 11 -1\n-1 -1 -3\n1 -1 -3\n0 1 -3", "0 1 2\n3 4 5");
    check(picker.pickTriangle(ordinal, forward) == 1, "triangle ordinal is not index offset");
    SculptMesh degenerate = meshFrom("0 0 -2\n1 0 -2\n2 0 -2", "0 1 2");
    check(!picker.pickTriangle(degenerate, forward), "degenerate triangle rejected");
    SculptMesh reversed = meshFrom("-1 -1 -2\n0 1 -2\n1 -1 -2", "0 1 2");
    check(picker.pickTriangle(reversed, forward) == 0, "back faces are pickable");
    SculptMesh invalid;
    check(!invalid.replaceFromText("0 0 0\n1 0 0", "0 1").success && !picker.pickTriangle(invalid, forward),
          "incomplete triples are rejected by the model and empty mesh is safe");

    TestCamera camera;
    const Viewport viewport{{10, 20}, {110, 120}};
    const auto center = makeCameraRay(camera, {60, 70}, viewport);
    check(center && near(center->direction.x, 0) && near(center->direction.y, 0) && center->direction.z < 0,
          "center ray points forward");
    const auto left = makeCameraRay(camera, {10, 70}, viewport);
    const auto right = makeCameraRay(camera, {110, 70}, viewport);
    check(left && right && left->direction.x < 0 && right->direction.x > 0, "left/right ray directions");
    const auto top = makeCameraRay(camera, {60, 20}, viewport);
    const auto bottom = makeCameraRay(camera, {60, 120}, viewport);
    check(top && bottom && top->direction.y > 0 && bottom->direction.y < 0, "top/bottom ray directions");
    check(!makeCameraRay(camera, {0, 0}, {{0, 0}, {0, 10}}), "zero-width viewport safely rejected");
    check(center && near(glm::length(center->direction), 1.0f), "camera ray direction normalized");
    const auto crosshair = makeCameraForwardRay(camera);
    check(crosshair && near(glm::length(crosshair->direction), 1.0f), "crosshair forward ray normalized");

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
