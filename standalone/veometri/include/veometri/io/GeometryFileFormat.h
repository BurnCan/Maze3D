#pragma once

#include <string>
#include <string_view>

#include "veometri/sculpt/SculptMesh.h"

namespace veometri::io {

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

} // namespace veometri::io
