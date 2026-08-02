#pragma once

#include <cstddef>
#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace mesh_sculpt::render { class Camera; }

namespace mesh_sculpt::sculpt {

class SculptMesh;

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Viewport
{
    glm::vec2 minimum;
    glm::vec2 maximum;
};

// Cursor/viewport coordinates use a top-left origin (screen Y increases down).
// The returned ray is in world space. Empty viewports or singular transforms
// produce no ray.
std::optional<Ray> makeCameraRay(const render::Camera& camera,
                                 const glm::vec2& cursorPosition,
                                 const Viewport& viewport) noexcept;

// Constructs the center-crosshair ray used by MeshSculptTool, preserving its
// historical position/forward convention without polling cursor state.
std::optional<Ray> makeCameraForwardRay(const render::Camera& camera) noexcept;

class MeshPicker
{
public:
    struct Settings
    {
        float vertexPickRadius = 0.10f;
        float minimumRayDistance = 0.0f;
        float epsilon = 1.0e-7f;
    };

    MeshPicker() = default;
    explicit MeshPicker(Settings settings) noexcept : m_settings(settings) {}

    std::optional<std::size_t> pickVertex(const SculptMesh& mesh, const Ray& ray) const noexcept;
    std::optional<std::size_t> pickTriangle(const SculptMesh& mesh, const Ray& ray) const noexcept;

private:
    Settings m_settings;
};

} // namespace mesh_sculpt::sculpt
