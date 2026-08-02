#pragma once

#include "veometri/sculpt/MeshEditorState.h"
#include <glm/glm.hpp>
#include <utility>

namespace veometri::render { class Camera; }
namespace veometri::sculpt {

class SculptMesh;
class MeshSelection;

struct MeshEditorAction
{
    enum class Type { None, ApplyMeshText, UpdateSelectedVertex, DeleteSelectedTriangle };
    Type type = Type::None;
    std::string verticesText;
    std::string indicesText;
    glm::vec3 vertexPosition{};
};

class VeometriEditorUi
{
public:
    MeshEditorAction renderToolPanel(const SculptMesh& mesh, const MeshSelection& selection);
    void renderOverlay(const render::Camera& camera, const SculptMesh& mesh,
                       const MeshSelection& selection, const glm::vec2& viewportMin,
                       const glm::vec2& viewportMax, bool drawCrosshair) const;
    void synchronizeVertices(const SculptMesh& mesh) { m_state.synchronizeVertices(mesh); }
    void synchronizeIndices(const SculptMesh& mesh) { m_state.synchronizeIndices(mesh); }
    void synchronizeAll(const SculptMesh& mesh) { m_state.synchronizeAll(mesh); }
    void setError(std::string message) { m_state.setError(std::move(message)); }
    void clearError() noexcept { m_state.clearError(); }
    const MeshEditorState& state() const noexcept { return m_state; }

private:
    MeshEditorState m_state;
};

} // namespace veometri::sculpt
