#include "veometri/sculpt/VeometriRenderData.h"
#include "veometri/sculpt/SculptMesh.h"

#include <cassert>

using veometri::sculpt::SculptMesh;
using veometri::sculpt::selectedTrianglePositions;
using veometri::sculpt::selectedVertexPosition;

int main()
{
    SculptMesh mesh = SculptMesh::makeDefaultCube();

    const auto triangle = selectedTrianglePositions(mesh, 0);
    assert(triangle);
    assert((*triangle)[0] == mesh.vertices()[mesh.indices()[0]]);
    assert((*triangle)[1] == mesh.vertices()[mesh.indices()[1]]);
    assert((*triangle)[2] == mesh.vertices()[mesh.indices()[2]]);
    assert(!selectedTrianglePositions(mesh, mesh.triangleCount()));

    const auto vertex = selectedVertexPosition(mesh, 0);
    assert(vertex && *vertex == mesh.vertices()[0]);
    assert(!selectedVertexPosition(mesh, mesh.vertexCount()));

    const SculptMesh empty;
    assert(!selectedTrianglePositions(empty, 0));
    assert(!selectedVertexPosition(empty, 0));

    const std::size_t deletedTriangle = mesh.triangleCount() - 1;
    assert(mesh.deleteTriangle(deletedTriangle));
    assert(!selectedTrianglePositions(mesh, deletedTriangle));
}
