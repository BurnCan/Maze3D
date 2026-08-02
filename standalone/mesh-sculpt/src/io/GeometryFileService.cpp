#include "mesh_sculpt/io/GeometryFileService.h"

#include "mesh_sculpt/io/GeometryFileFormat.h"

#include <fstream>
#include <system_error>

namespace mesh_sculpt::io {
namespace {
std::string displayPath(const std::filesystem::path& path) { return path.string(); }
}

GeometryFileService::LoadResult GeometryFileService::load(const std::filesystem::path& path)
{
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
        return {false, {}, "Could not open \"" + displayPath(path) + "\" for reading."};
    const auto size = std::filesystem::file_size(path, error);
    if (error) return {false, {}, "Could not determine geometry file size."};
    if (size > maximumFileSize)
        return {false, {}, "Geometry file exceeds the 64 MiB size limit."};
    std::ifstream input(path, std::ios::binary);
    if (!input) return {false, {}, "Could not open \"" + displayPath(path) + "\" for reading."};
    std::string text(static_cast<std::size_t>(size), '\0');
    input.read(text.data(), static_cast<std::streamsize>(text.size()));
    if (!input && !input.eof()) return {false, {}, "Could not read complete geometry file."};
    auto decoded = GeometryFileFormat::decode(text);
    return {decoded.success, std::move(decoded.mesh), std::move(decoded.error)};
}

GeometryFileService::SaveResult GeometryFileService::save(
    const std::filesystem::path& path, const sculpt::SculptMesh& mesh)
{
    std::string text;
    try { text = GeometryFileFormat::encode(mesh); }
    catch (const std::exception& error) { return {false, error.what()}; }
    auto temporary = path;
    temporary += ".tmp";
    std::error_code filesystemError;
    std::filesystem::remove(temporary, filesystemError);
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) return {false, "Could not open temporary file for writing."};
        output.write(text.data(), static_cast<std::streamsize>(text.size()));
        output.flush();
        if (!output)
        {
            output.close();
            std::filesystem::remove(temporary, filesystemError);
            return {false, "Could not write and flush complete geometry file."};
        }
        output.close();
        if (!output)
        {
            std::filesystem::remove(temporary, filesystemError);
            return {false, "Could not close temporary geometry file."};
        }
    }
    // POSIX rename replaces atomically. On platforms that reject replacement,
    // retain the valid destination rather than deleting it first.
    std::filesystem::rename(temporary, path, filesystemError);
    if (filesystemError)
    {
        std::filesystem::remove(temporary, filesystemError);
        return {false, "Could not replace destination file."};
    }
    return {true, {}};
}

} // namespace mesh_sculpt::io
