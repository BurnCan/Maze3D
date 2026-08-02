#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <glm/vec3.hpp>

namespace mesh_sculpt::sculpt {

class SculptMesh
{
public:
    struct ParseResult
    {
        bool success = false;
        std::string error;
    };
    struct CreateResult;

    SculptMesh() = default;
    static SculptMesh makeDefaultCube();
    static CreateResult create(std::vector<glm::vec3> vertices,
                               std::vector<std::uint32_t> indices);

    const std::vector<glm::vec3>& vertices() const noexcept { return m_vertices; }
    const std::vector<std::uint32_t>& indices() const noexcept { return m_indices; }

    std::size_t vertexCount() const noexcept { return m_vertices.size(); }
    std::size_t triangleCount() const noexcept { return m_indices.size() / 3; }
    bool empty() const noexcept { return m_vertices.empty(); }

    ParseResult replaceFromText(std::string_view verticesText, std::string_view indicesText);
    std::string verticesToText() const;
    std::string indicesToText() const;

    bool setVertex(std::size_t index, const glm::vec3& position);
    bool deleteTriangle(std::size_t triangleIndex);

    bool isValid() const noexcept;
    std::string validationError() const;
    bool equals(const SculptMesh& other) const noexcept;

private:
    std::vector<glm::vec3> m_vertices;
    std::vector<std::uint32_t> m_indices;
};

struct SculptMesh::CreateResult
{
    bool success = false;
    SculptMesh mesh;
    std::string error;
};

} // namespace mesh_sculpt::sculpt
