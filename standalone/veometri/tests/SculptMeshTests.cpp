#include "veometri/sculpt/SculptMesh.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using veometri::sculpt::SculptMesh;

namespace {
int failures = 0;

void check(bool condition, const char* description)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << description << '\n';
        ++failures;
    }
}
} // namespace

int main()
{
    SculptMesh cube = SculptMesh::makeDefaultCube();
    check(cube.vertexCount() == 8, "default cube vertex count");
    check(cube.triangleCount() == 12, "default cube triangle count");
    check(cube.isValid(), "default cube validity");

    SculptMesh mesh;
    const auto valid = mesh.replaceFromText("0 0 0\n1 0 0\n0 1 0\n", "0 1 2\n");
    check(valid.success && mesh.vertexCount() == 3 && mesh.triangleCount() == 1,
          "valid vertex/index text parsing");
    check(mesh.verticesToText() == "0 0 0\n1 0 0\n0 1 0\n" &&
          mesh.indicesToText() == "0 1 2\n", "deterministic serialization");

    check(!mesh.replaceFromText("0 0", "0 1 2").success, "malformed vertex text rejection");
    check(!mesh.replaceFromText("0 0 0\n1 0 0\n0 1 0", "0 abc 2").success,
          "malformed index text rejection");
    check(!mesh.replaceFromText("0 0 0", "0 0 -1").success,
          "negative index rejection");
    check(!mesh.replaceFromText("0 0 0", "0 0").success,
          "non-triangle index count rejection");
    check(!mesh.replaceFromText("0 0 0", "0 0 1").success,
          "out-of-range index rejection");
    check(mesh.vertexCount() == 3 && mesh.indicesToText() == "0 1 2\n",
          "failed parsing preserves previous mesh");

    check(mesh.setVertex(1, {2.0f, 3.0f, 4.0f}) && mesh.vertices()[1].x == 2.0f,
          "valid vertex mutation");
    const std::string beforeInvalidVertex = mesh.verticesToText();
    check(!mesh.setVertex(9, {1.0f, 1.0f, 1.0f}) &&
          mesh.verticesToText() == beforeInvalidVertex, "invalid vertex mutation");

    SculptMesh deletionMesh;
    check(deletionMesh.replaceFromText("0 0 0\n1 0 0\n0 1 0\n1 1 0", "0 1 2 2 1 3 3 1 0").success,
          "triangle deletion fixture parsing");
    check(deletionMesh.deleteTriangle(1) && deletionMesh.triangleCount() == 2,
          "valid triangle deletion");
    check(deletionMesh.indicesToText() == "0 1 2\n3 1 0\n",
          "triangle deletion preserves remaining order");
    const std::string beforeInvalidDeletion = deletionMesh.indicesToText();
    check(!deletionMesh.deleteTriangle(8) && deletionMesh.indicesToText() == beforeInvalidDeletion,
          "invalid triangle deletion");

    if (failures == 0)
        std::cout << "All SculptMesh tests passed.\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
