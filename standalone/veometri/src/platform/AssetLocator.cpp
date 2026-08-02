#include "veometri/platform/AssetLocator.h"

#include <mutex>
#include <sstream>
#include <stdexcept>

namespace veometri::platform {
namespace {
std::filesystem::path executablePath;
std::mutex pathMutex;
}

void AssetLocator::setExecutablePath(const std::filesystem::path& path)
{
    std::lock_guard lock(pathMutex);
    executablePath = std::filesystem::absolute(path);
}

std::vector<std::filesystem::path> AssetLocator::candidateRoots(const std::filesystem::path& executable)
{
    const auto directory = std::filesystem::absolute(executable).parent_path();
    return {directory / "assets", directory.parent_path() / "share" / "veometri",
        std::filesystem::path(VEOMETRI_DEVELOPMENT_ASSET_ROOT)};
}

std::filesystem::path AssetLocator::resolve(const std::filesystem::path& relativePath)
{
    std::filesystem::path executable;
    {
        std::lock_guard lock(pathMutex);
        executable = executablePath;
    }
    if (executable.empty())
        throw std::runtime_error("Asset lookup attempted before the executable path was registered");
    std::ostringstream attempted;
    for (const auto& root : candidateRoots(executable))
    {
        const auto candidate = root / relativePath;
        attempted << "\n  - " << candidate.string();
        if (std::filesystem::is_regular_file(candidate)) return candidate;
    }
    throw std::runtime_error("Unable to locate asset '" + relativePath.string() + "'. Tried:" + attempted.str());
}
} // namespace veometri::platform
