#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "veometri/sculpt/SculptMesh.h"

namespace veometri::io {

class GeometryFileService
{
public:
    static constexpr std::uintmax_t maximumFileSize = 64U * 1024U * 1024U;

    struct LoadResult
    {
        bool success = false;
        sculpt::SculptMesh mesh;
        std::string error;
    };
    struct SaveResult
    {
        bool success = false;
        std::string error;
    };

    static LoadResult load(const std::filesystem::path& path);
    static SaveResult save(const std::filesystem::path& path,
                           const sculpt::SculptMesh& mesh);
};

} // namespace veometri::io
