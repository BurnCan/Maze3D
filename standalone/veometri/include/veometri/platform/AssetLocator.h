#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace veometri::platform {

class AssetLocator
{
public:
    static void setExecutablePath(const std::filesystem::path& path);
    static std::vector<std::filesystem::path> candidateRoots(const std::filesystem::path& executable);
    static std::filesystem::path resolve(const std::filesystem::path& relativePath);
};

} // namespace veometri::platform
