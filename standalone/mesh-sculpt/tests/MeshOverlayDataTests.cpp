#include "mesh_sculpt/sculpt/MeshOverlayData.h"
#include "mesh_sculpt/sculpt/MeshSelection.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <iostream>

using namespace mesh_sculpt::sculpt;
namespace { int failures; void check(bool value, const char* message) { if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; } } }

int main()
{
    SculptMesh mesh;
    check(mesh.replaceFromText("0 0 -2\n0.5 0 -2\n0 0.5 -2\n0 0 2", "0 1 2").success, "fixture");
    const glm::mat4 view(1.0f);
    const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::vec2 min(0.0f), max(100.0f);
    MeshSelection selection;

    selection.selectVertex(0);
    auto labels = makeMeshOverlayLabels(view, projection, mesh, selection, min, max);
    check(labels.size() == 1 && labels[0].vertexIndex == 0, "selected vertex label");
    check(labels[0].screenPosition == glm::vec2(50.0f), "expected center projection");
    selection.selectTriangle(0);
    labels = makeMeshOverlayLabels(view, projection, mesh, selection, min, max);
    check(labels.size() == 3 && labels[0].vertexIndex == 0 && labels[2].vertexIndex == 2,
          "triangle labels expected vertices");
    selection.clear();
    check(makeMeshOverlayLabels(view, projection, mesh, selection, min, max).empty(), "empty selection");
    selection.selectVertex(3);
    check(makeMeshOverlayLabels(view, projection, mesh, selection, min, max).empty(), "behind camera omitted");
    selection.selectVertex(0);
    check(makeMeshOverlayLabels(view, projection, mesh, selection, min, min).empty(), "zero viewport safe");
    selection.selectVertex(99);
    check(makeMeshOverlayLabels(view, projection, mesh, selection, min, max).empty(), "stale selection safe");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
