#include "veometri/io/GeometryFileFormat.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

#include <nlohmann/json.hpp>

namespace veometri::io {
namespace {
using Json = nlohmann::ordered_json;
constexpr std::size_t maximumVertices = 5'000'000;
constexpr std::size_t maximumIndices = 15'000'000;

std::string requireField(const Json& root, const char* name)
{
    return root.contains(name) ? std::string{} :
        "Missing required field \"" + std::string(name) + "\".";
}
}

std::string GeometryFileFormat::encode(const sculpt::SculptMesh& mesh)
{
    if (!mesh.isValid())
        throw std::invalid_argument("Cannot encode invalid geometry: " + mesh.validationError());
    Json root = Json::object();
    root["format"] = "indexed-geometry";
    root["version"] = 1;
    root["primitive"] = "triangles";
    root["vertices"] = Json::array();
    for (const auto& vertex : mesh.vertices())
    {
        if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y) || !std::isfinite(vertex.z))
            throw std::invalid_argument("Cannot encode a non-finite vertex coordinate.");
        root["vertices"].push_back({vertex.x, vertex.y, vertex.z});
    }
    root["indices"] = mesh.indices();
    return root.dump(2) + '\n';
}

GeometryFileFormat::DecodeResult GeometryFileFormat::decode(std::string_view text)
{
    Json root;
    try { root = Json::parse(text.begin(), text.end()); }
    catch (const Json::parse_error& error)
    {
        return {false, {}, "Malformed JSON: " + std::string(error.what())};
    }
    if (!root.is_object()) return {false, {}, "Expected top-level JSON object."};
    for (const char* field : {"format", "version", "primitive", "vertices", "indices"})
        if (auto error = requireField(root, field); !error.empty()) return {false, {}, error};
    if (!root["format"].is_string()) return {false, {}, "Field \"format\" must be a string."};
    if (root["format"] != "indexed-geometry")
        return {false, {}, "Unsupported geometry format \"" + root["format"].get<std::string>() + "\"."};
    if (!root["version"].is_number_integer()) return {false, {}, "Field \"version\" must be an integer."};
    if (root["version"] != 1)
        return {false, {}, "Unsupported geometry version " + root["version"].dump() + "."};
    if (!root["primitive"].is_string()) return {false, {}, "Field \"primitive\" must be a string."};
    if (root["primitive"] != "triangles") return {false, {}, "Unsupported geometry primitive."};
    if (!root["vertices"].is_array()) return {false, {}, "Field \"vertices\" must be an array."};
    if (!root["indices"].is_array()) return {false, {}, "Field \"indices\" must be an array."};
    if (root["vertices"].size() > maximumVertices)
        return {false, {}, "Vertex count exceeds the limit of 5000000."};
    if (root["indices"].size() > maximumIndices)
        return {false, {}, "Index count exceeds the limit of 15000000."};

    std::vector<glm::vec3> vertices;
    vertices.reserve(root["vertices"].size());
    for (std::size_t i = 0; i < root["vertices"].size(); ++i)
    {
        const auto& tuple = root["vertices"][i];
        if (!tuple.is_array() || tuple.size() != 3)
            return {false, {}, "Vertex " + std::to_string(i) + " must contain exactly three components."};
        float values[3];
        for (std::size_t component = 0; component < 3; ++component)
        {
            if (!tuple[component].is_number())
                return {false, {}, "Vertex " + std::to_string(i) + " contains a nonnumeric coordinate."};
            const double value = tuple[component].get<double>();
            if (!std::isfinite(value) || value > std::numeric_limits<float>::max() ||
                value < -std::numeric_limits<float>::max())
                return {false, {}, "Vertex " + std::to_string(i) + " contains a non-finite coordinate."};
            values[component] = static_cast<float>(value);
        }
        vertices.emplace_back(values[0], values[1], values[2]);
    }
    if (root["indices"].size() % 3 != 0)
        return {false, {}, "Triangle index count must be divisible by three."};
    std::vector<std::uint32_t> indices;
    indices.reserve(root["indices"].size());
    for (std::size_t i = 0; i < root["indices"].size(); ++i)
    {
        const auto& item = root["indices"][i];
        if (!item.is_number_unsigned())
            return {false, {}, "Index " + std::to_string(i) + " is not an unsigned integer."};
        const auto value = item.get<std::uint64_t>();
        if (value > std::numeric_limits<std::uint32_t>::max())
            return {false, {}, "Index " + std::to_string(i) + " exceeds the application index type."};
        if (value >= vertices.size())
            return {false, {}, "Index " + std::to_string(i) + " references missing vertex " +
                std::to_string(value) + "."};
        indices.push_back(static_cast<std::uint32_t>(value));
    }
    auto created = sculpt::SculptMesh::create(std::move(vertices), std::move(indices));
    if (!created.success) return {false, {}, created.error};
    return {true, std::move(created.mesh), {}};
}

} // namespace veometri::io
