#include "mesh_sculpt/sculpt/MeshEditorState.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"
#include <cstdlib>
#include <iostream>

using namespace mesh_sculpt::sculpt;
namespace { int failures; void check(bool value, const char* message) { if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; } } }

int main()
{
    SculptMesh cube = SculptMesh::makeDefaultCube();
    MeshEditorState state;
    state.synchronizeAll(cube);
    check(state.verticesText() == cube.verticesToText(), "default vertices initialize");
    check(state.indicesText() == cube.indicesToText(), "default indices initialize");

    state.editIndicesText() = "unfinished indices";
    state.synchronizeVertices(cube);
    check(state.indicesText() == "unfinished indices", "vertex sync preserves index edits");
    state.editVerticesText() = "unfinished vertices";
    state.synchronizeIndices(cube);
    check(state.verticesText() == "unfinished vertices", "index sync preserves vertex edits");

    state.editVerticesText() = "invalid";
    state.setError("Invalid vertex data");
    check(state.verticesText() == "invalid", "error preserves invalid input");
    check(state.errorMessage() == "Invalid vertex data", "error can be set");
    state.clearError();
    state.synchronizeAll(cube);
    check(state.errorMessage().empty(), "success clears error");
    check(state.verticesText() == cube.verticesToText() && state.indicesText() == cube.indicesToText(),
          "full sync restores canonical serialization");

    state.editVerticesText() = std::string(9000, '1');
    check(state.verticesText().size() == 9000, "text larger than old buffer is retained");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
