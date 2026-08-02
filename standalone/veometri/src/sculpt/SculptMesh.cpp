#include "veometri/sculpt/SculptMesh.h"

#include <cerrno>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <sstream>
#include <utility>

namespace veometri::sculpt {
namespace {

std::vector<std::string> tokens(std::string_view text)
{
    std::istringstream input{std::string(text)};
    std::vector<std::string> result;
    for (std::string token; input >> token;)
        result.push_back(std::move(token));
    return result;
}

bool parseFloat(const std::string& token, float& value)
{
    char* end = nullptr;
    errno = 0;
    value = std::strtof(token.c_str(), &end);
    return end != token.c_str() && *end == '\0' && errno != ERANGE;
}

bool parseIndex(const std::string& token, std::uint32_t& value)
{
    if (token.empty() || token.front() == '-')
        return false;
    char* end = nullptr;
    errno = 0;
    const unsigned long long parsed = std::strtoull(token.c_str(), &end, 10);
    if (end == token.c_str() || *end != '\0' || errno == ERANGE ||
        parsed > std::numeric_limits<std::uint32_t>::max())
        return false;
    value = static_cast<std::uint32_t>(parsed);
    return true;
}

std::string validationError(const std::vector<glm::vec3>& vertices,
                            const std::vector<std::uint32_t>& indices)
{
    for (std::size_t i = 0; i < vertices.size(); ++i)
        if (!std::isfinite(vertices[i].x) || !std::isfinite(vertices[i].y) ||
            !std::isfinite(vertices[i].z))
            return "Vertex " + std::to_string(i) + " contains a non-finite coordinate.";
    if (indices.size() % 3 != 0)
        return "Triangle index data must contain a multiple of three values.";
    for (std::size_t i = 0; i < indices.size(); ++i)
        if (indices[i] >= vertices.size())
            return "Index " + std::to_string(i) + " references missing vertex " +
                   std::to_string(indices[i]) + ".";
    return {};
}

} // namespace

SculptMesh::CreateResult SculptMesh::create(std::vector<glm::vec3> vertices,
                                             std::vector<std::uint32_t> indices)
{
    if (auto error = ::veometri::sculpt::validationError(vertices, indices); !error.empty())
        return {false, {}, std::move(error)};
    SculptMesh mesh;
    mesh.m_vertices = std::move(vertices);
    mesh.m_indices = std::move(indices);
    return {true, std::move(mesh), {}};
}

SculptMesh
SculptMesh::makeDefaultCube()
{
    SculptMesh mesh;
    mesh.m_vertices = {
        {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
        {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}
    };
    mesh.m_indices = {
        0,1,2, 2,3,0, 4,5,6, 6,7,4, 0,4,7, 7,3,0,
        1,5,6, 6,2,1, 3,7,6, 6,2,3, 0,1,5, 5,4,0
    };
    return mesh;
}

SculptMesh::ParseResult SculptMesh::replaceFromText(std::string_view verticesText,
                                                     std::string_view indicesText)
{
    const auto vertexTokens = tokens(verticesText);
    const auto indexTokens = tokens(indicesText);
    if (vertexTokens.empty())
        return {false, "Vertex data must contain at least one vertex."};
    if (indexTokens.empty())
        return {false, "Index data must contain at least one triangle."};
    if (vertexTokens.size() % 3 != 0)
    {
        const std::size_t vertex = vertexTokens.size() / 3;
        static constexpr const char* components[] = {"X", "Y", "Z"};
        return {false, "Vertex " + std::to_string(vertex) + " is missing a " +
                       components[vertexTokens.size() % 3] + " component."};
    }

    std::vector<glm::vec3> parsedVertices;
    parsedVertices.reserve(vertexTokens.size() / 3);
    for (std::size_t i = 0; i < vertexTokens.size(); i += 3)
    {
        float components[3];
        for (std::size_t component = 0; component < 3; ++component)
            if (!parseFloat(vertexTokens[i + component], components[component]))
                return {false, "Vertex " + std::to_string(i / 3) + " component " +
                               std::string("XYZ").substr(component, 1) + " token \"" +
                               vertexTokens[i + component] + "\" is not a number."};
        parsedVertices.emplace_back(components[0], components[1], components[2]);
    }

    std::vector<std::uint32_t> parsedIndices;
    parsedIndices.reserve(indexTokens.size());
    for (std::size_t i = 0; i < indexTokens.size(); ++i)
    {
        std::uint32_t index = 0;
        if (!parseIndex(indexTokens[i], index))
            return {false, "Index token \"" + indexTokens[i] + "\" at position " +
                           std::to_string(i) + " is not an unsigned integer."};
        parsedIndices.push_back(index);
    }

    if (const std::string error = ::veometri::sculpt::validationError(parsedVertices, parsedIndices);
        !error.empty())
        return {false, error};

    m_vertices = std::move(parsedVertices);
    m_indices = std::move(parsedIndices);
    return {true, {}};
}

std::string SculptMesh::verticesToText() const
{
    std::ostringstream output;
    for (const auto& vertex : m_vertices)
        output << vertex.x << ' ' << vertex.y << ' ' << vertex.z << '\n';
    return output.str();
}

std::string SculptMesh::indicesToText() const
{
    std::ostringstream output;
    for (std::size_t i = 0; i < m_indices.size(); i += 3)
        output << m_indices[i] << ' ' << m_indices[i + 1] << ' ' << m_indices[i + 2] << '\n';
    return output.str();
}

bool SculptMesh::setVertex(std::size_t index, const glm::vec3& position)
{
    if (index >= m_vertices.size() || !std::isfinite(position.x) ||
        !std::isfinite(position.y) || !std::isfinite(position.z))
        return false;
    m_vertices[index] = position;
    return true;
}

bool SculptMesh::deleteTriangle(std::size_t triangleIndex)
{
    if (triangleIndex >= triangleCount())
        return false;
    const auto first = m_indices.begin() + static_cast<std::ptrdiff_t>(triangleIndex * 3);
    m_indices.erase(first, first + 3);
    return true;
}

bool SculptMesh::isValid() const noexcept
{
    if (m_indices.size() % 3 != 0)
        return false;
    for (const auto index : m_indices)
        if (index >= m_vertices.size())
            return false;
    return true;
}

std::string SculptMesh::validationError() const
{
    return ::veometri::sculpt::validationError(m_vertices, m_indices);
}

bool SculptMesh::equals(const SculptMesh& other) const noexcept
{
    if (m_vertices.size() != other.m_vertices.size() || m_indices != other.m_indices)
        return false;
    for (std::size_t i = 0; i < m_vertices.size(); ++i)
    {
        const auto& left = m_vertices[i];
        const auto& right = other.m_vertices[i];
        if (left.x != right.x || left.y != right.y || left.z != right.z)
            return false;
    }
    return true;
}

} // namespace veometri::sculpt
