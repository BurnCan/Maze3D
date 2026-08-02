#pragma once

#include <string>
#include <string_view>

#include "mesh_sculpt/sculpt/SculptMesh.h"

namespace mesh_sculpt::io {

class GeometryFileFormat
{
public:
    struct DecodeResult
    {
        bool success = false;
        sculpt::SculptMesh mesh;
        std::string error;
    };

    static std::string encode(const sculpt::SculptMesh& mesh);
    static DecodeResult decode(std::string_view text);
};

} // namespace mesh_sculpt::io
