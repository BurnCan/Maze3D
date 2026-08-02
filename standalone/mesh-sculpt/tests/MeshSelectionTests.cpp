#include "mesh_sculpt/sculpt/MeshSelection.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"

#include <cstdlib>
#include <iostream>

using namespace mesh_sculpt::sculpt;

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
}

int main()
{
    SculptMesh mesh;
    check(mesh.replaceFromText("0 0 0\n1 0 0\n0 1 0", "0 1 2").success, "fixture");
    MeshSelection selection;
    check(!selection.selectedVertex() && !selection.selectedTriangle(), "default is empty");
    selection.selectVertex(1);
    check(selection.selectedVertex() == 1, "select vertex");
    selection.selectTriangle(0);
    check(selection.selectedTriangle() == 0, "select triangle");
    check(selection.selectedVertex() == 1, "vertex and triangle may coexist");
    selection.validateAgainst(mesh);
    check(selection.selectedVertex() == 1 && selection.selectedTriangle() == 0, "validation preserves valid selections");
    selection.clearVertex();
    check(!selection.selectedVertex() && selection.selectedTriangle() == 0, "clear vertex only");
    selection.selectVertex(2);
    selection.clearTriangle();
    check(selection.selectedVertex() == 2 && !selection.selectedTriangle(), "clear triangle only");
    selection.selectTriangle(0);
    selection.clear();
    check(!selection.selectedVertex() && !selection.selectedTriangle(), "clear all");
    selection.selectVertex(3);
    selection.selectTriangle(1);
    selection.validateAgainst(mesh);
    check(!selection.selectedVertex(), "out-of-range vertex cleared");
    check(!selection.selectedTriangle(), "out-of-range triangle cleared");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
