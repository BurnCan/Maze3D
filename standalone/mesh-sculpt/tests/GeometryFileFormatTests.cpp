#include "mesh_sculpt/io/GeometryFileFormat.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>

using mesh_sculpt::io::GeometryFileFormat;
using mesh_sculpt::sculpt::SculptMesh;
namespace { int failures; void check(bool value, const char* message) { if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; } } }

int main()
{
    const auto cube = SculptMesh::makeDefaultCube();
    const auto encoded = GeometryFileFormat::encode(cube);
    check(!encoded.empty() && encoded.back() == '\n', "cube encodes with final newline");
    check(encoded == GeometryFileFormat::encode(cube), "encoding is deterministic");
    check(encoded.find("\"format\": \"indexed-geometry\"") != std::string::npos, "format identifier present");
    check(encoded.find("\"version\": 1") != std::string::npos, "version present");
    auto decoded = GeometryFileFormat::decode(encoded);
    check(decoded.success && decoded.mesh.equals(cube), "round trip preserves vertices, indices, and winding");
    check(decoded.mesh.isValid(), "decoded geometry satisfies invariants");
    std::ifstream example(MESH_SCULPT_EXAMPLE_FILE);
    const std::string exampleText((std::istreambuf_iterator<char>(example)), {});
    check(example.good() || example.eof(), "example geometry is readable");
    const auto exampleDecoded = GeometryFileFormat::decode(exampleText);
    check(exampleDecoded.success && exampleDecoded.mesh.equals(cube), "example cube loads successfully");
    check(GeometryFileFormat::decode(R"({"format":"indexed-geometry","version":1,"primitive":"triangles","vertices":[],"indices":[]})").success,
          "valid empty mesh round trips");
    check(!GeometryFileFormat::decode("{").success, "malformed JSON rejected");
    check(!GeometryFileFormat::decode(R"({"version":1,"primitive":"triangles","vertices":[],"indices":[]})").success, "missing format rejected");
    check(!GeometryFileFormat::decode(R"({"format":"other","version":1,"primitive":"triangles","vertices":[],"indices":[]})").success, "unsupported format rejected");
    check(!GeometryFileFormat::decode(R"({"format":"indexed-geometry","primitive":"triangles","vertices":[],"indices":[]})").success, "missing version rejected");
    check(!GeometryFileFormat::decode(R"({"format":"indexed-geometry","version":3,"primitive":"triangles","vertices":[],"indices":[]})").success, "unsupported version rejected");
    check(!GeometryFileFormat::decode(R"({"format":"indexed-geometry","version":1,"primitive":"lines","vertices":[],"indices":[]})").success, "unsupported primitive rejected");
    const std::string prefix = R"({"format":"indexed-geometry","version":1,"primitive":"triangles","vertices":)";
    check(!GeometryFileFormat::decode(prefix + R"([[0,0]] ,"indices":[]})").success, "malformed vertex tuple rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[0,"x",0]],"indices":[0,0,0]})").success, "nonnumeric coordinate rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[1e400,0,0]],"indices":[0,0,0]})").success, "non-finite coordinate rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[0,0,0]],"indices":[-1,0,0]})").success, "negative index rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[0,0,0]],"indices":[0.5,0,0]})").success, "fractional index rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[0,0,0]],"indices":[4294967296,0,0]})").success, "index overflow rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[0,0,0]],"indices":[0]})").success, "non-triangle index count rejected");
    check(!GeometryFileFormat::decode(prefix + R"([[0,0,0]],"indices":[0,1,0]})").success, "missing vertex reference rejected");
    check(GeometryFileFormat::decode(prefix + R"([],"indices":[],"future":true})").success, "unknown top-level fields tolerated");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
