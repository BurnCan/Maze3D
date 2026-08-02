#pragma once

#include <optional>

#include <glm/vec3.hpp>

#include "veometri/sculpt/MeshPicker.h"

namespace veometri::sculpt {

// Owns the geometry and lifetime of a free vertex drag. The drag plane uses
// dot(normal, point) = distance and intentionally applies no grab offset,
// preserving VeometriTool's historical snap-to-intersection behavior.
class VertexDragManipulator
{
public:
    struct Settings
    {
        float epsilon = 1.0e-5f;
    };

    VertexDragManipulator() = default;
    explicit VertexDragManipulator(Settings settings) noexcept;

    // A begin always replaces the previous drag. Failure leaves the
    // manipulator inactive.
    bool begin(const glm::vec3& vertexPosition,
               const glm::vec3& dragPlaneNormal,
               const Ray& initialRay) noexcept;

    std::optional<glm::vec3> update(const Ray& currentRay) const noexcept;
    void end() noexcept;
    bool isActive() const noexcept;

private:
    Settings m_settings;
    bool m_active = false;
    glm::vec3 m_planeNormal{0.0f};
    float m_planeDistance = 0.0f;
};

} // namespace veometri::sculpt
