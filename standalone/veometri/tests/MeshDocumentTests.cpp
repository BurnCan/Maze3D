#include "veometri/sculpt/MeshDocument.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace veometri::sculpt;

namespace {
int failures = 0;
void check(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
}

int main()
{
    MeshDocument document;
    const SculptMesh defaultCube = SculptMesh::makeDefaultCube();
    check(document.mesh().isValid() && document.mesh().equals(defaultCube), "initial default cube is valid");
    check(!document.isDirty(), "initial document is clean");
    check(document.revision() == 0, "initial revision is zero");
    check(!document.selection().selectedVertex() && !document.selection().selectedTriangle(), "initial selection is empty");

    const auto originalVertex = document.mesh().vertices()[0];
    auto result = document.setVertex(0, {2.0f, 3.0f, 4.0f});
    check(result.success && result.geometryChanged && result.verticesChanged && !result.indicesChanged,
          "valid vertex change reports vertex geometry change");
    check(document.revision() == 1 && document.isDirty(), "vertex change increments revision and marks dirty");
    result = document.setVertex(0, {2.0f, 3.0f, 4.0f});
    check(result.success && !result.geometryChanged && document.revision() == 1, "same vertex is unchanged");
    const SculptMesh beforeInvalidVertex = document.mesh();
    result = document.setVertex(999, {});
    check(!result.success && !result.geometryChanged && document.mesh().equals(beforeInvalidVertex) &&
          document.revision() == 1, "invalid vertex is transactional");

    document.selectVertex(0);
    document.selectTriangle(1);
    result = document.deleteTriangle(1);
    check(result.success && result.geometryChanged && !result.verticesChanged && result.indicesChanged,
          "triangle deletion changes only indices");
    check(document.revision() == 2 && !document.selection().selectedTriangle() &&
          document.selection().selectedVertex() == 0, "deletion increments revision and preserves valid vertex");
    const SculptMesh beforeInvalidTriangle = document.mesh();
    result = document.deleteTriangle(999);
    check(!result.success && document.mesh().equals(beforeInvalidTriangle) && document.revision() == 2,
          "invalid triangle deletion is transactional");

    const std::string replacementVertices = "0 0 0\n1 0 0\n0 1 0\n";
    const std::string replacementIndices = "0 1 2\n";
    document.selectVertex(0);
    result = document.replaceFromText(replacementVertices, replacementIndices);
    check(result.success && result.geometryChanged && result.verticesChanged && result.indicesChanged,
          "valid replacement reports changes");
    check(!document.selection().selectedVertex() && !document.selection().selectedTriangle(),
          "replacement clears selection");
    const SculptMesh beforeInvalidText = document.mesh();
    const auto revisionBeforeInvalidText = document.revision();
    const bool dirtyBeforeInvalidText = document.isDirty();
    result = document.replaceFromText("bad", replacementIndices);
    check(!result.success && document.mesh().equals(beforeInvalidText) &&
          document.revision() == revisionBeforeInvalidText && document.isDirty() == dirtyBeforeInvalidText,
          "invalid replacement preserves mesh, revision, and dirty state");
    result = document.replaceFromText(replacementVertices, replacementIndices);
    check(result.success && !result.geometryChanged && document.revision() == revisionBeforeInvalidText,
          "identical replacement reports no change");

    result = document.replaceFromText("0 0 0\n2 0 0\n0 1 0\n", replacementIndices);
    check(result.success && result.verticesChanged && !result.indicesChanged, "vertex-only replacement flags");
    result = document.replaceFromText("0 0 0\n2 0 0\n0 1 0\n", "0 2 1\n");
    check(result.success && !result.verticesChanged && result.indicesChanged, "index-only replacement flags");

    document.selectVertex(0);
    result = document.resetToDefault();
    check(result.success && result.geometryChanged && document.mesh().equals(defaultCube) &&
          !document.selection().selectedVertex(), "reset restores cube and clears selection");
    const auto resetRevision = document.revision();
    result = document.resetToDefault();
    check(result.success && !result.geometryChanged && document.revision() == resetRevision,
          "reset at default reports no change");

    document.setVertex(0, {3.0f, 3.0f, 3.0f});
    document.markSaved();
    check(!document.isDirty(), "markSaved establishes clean snapshot");
    const SculptMesh saved = document.mesh();
    document.setVertex(0, originalVertex);
    check(document.isDirty(), "later mutation marks dirty");
    document.setVertex(0, saved.vertices()[0]);
    check(!document.isDirty(), "returning to saved geometry clears dirty state");
    const auto beforeSelectionRevision = document.revision();
    check(document.selectVertex(1), "valid vertex selection succeeds");
    check(!document.selectVertex(999), "invalid vertex selection fails");
    check(document.selectTriangle(0), "valid triangle selection succeeds");
    check(!document.selectTriangle(999), "invalid triangle selection fails");
    check(document.selection().selectedVertex() == 1 && document.selection().selectedTriangle() == 0,
          "existing independent vertex and triangle selections coexist");
    check(!document.isDirty() && document.revision() == beforeSelectionRevision,
          "selection does not affect dirty state or revision");
    document.clearSelection();
    document.clearSelection();
    check(!document.selection().selectedVertex() && !document.selection().selectedTriangle(),
          "clear selection is idempotent");

    MeshDocument loadedDocument;
    loadedDocument.selectVertex(0);
    auto created = SculptMesh::create({{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}, {0, 2, 1});
    result = loadedDocument.replaceMesh(std::move(created.mesh));
    check(result.success && result.geometryChanged && result.verticesChanged && result.indicesChanged,
          "replaceMesh reports changed geometry");
    check(!loadedDocument.selection().selectedVertex() && loadedDocument.revision() == 1,
          "load-style replacement clears selection and increments revision");
    loadedDocument.markSaved();
    check(!loadedDocument.isDirty(), "markSaved after load clears dirty state");
    const auto loadedRevision = loadedDocument.revision();
    result = loadedDocument.replaceMesh(loadedDocument.mesh());
    check(result.success && !result.geometryChanged && loadedDocument.revision() == loadedRevision,
          "identical replaceMesh does not increment revision");

    if (failures == 0) std::cout << "All MeshDocument tests passed.\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
