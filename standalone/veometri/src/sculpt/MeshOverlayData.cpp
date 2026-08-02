#include "veometri/sculpt/MeshOverlayData.h"
#include "veometri/sculpt/MeshSelection.h"
#include "veometri/sculpt/SculptMesh.h"

namespace veometri::sculpt {

std::vector<VertexLabel> makeMeshOverlayLabels(
    const glm::mat4& view, const glm::mat4& projection,
    const SculptMesh& mesh, const MeshSelection& selection,
    const glm::vec2& viewportMin, const glm::vec2& viewportMax)
{
    std::vector<VertexLabel> labels;
    const glm::vec2 size = viewportMax - viewportMin;
    if (size.x <= 0.0f || size.y <= 0.0f)
        return labels;

    std::vector<std::size_t> vertexIndices;
    if (const auto triangle = selection.selectedTriangle())
    {
        const std::size_t base = *triangle * 3;
        if (base + 2 >= mesh.indices().size())
            return labels;
        vertexIndices = {mesh.indices()[base], mesh.indices()[base + 1], mesh.indices()[base + 2]};
    }
    else if (const auto vertex = selection.selectedVertex())
    {
        vertexIndices = {*vertex};
    }

    for (const std::size_t index : vertexIndices)
    {
        if (index >= mesh.vertices().size())
            continue;
        const glm::vec4 clip = projection * view * glm::vec4(mesh.vertices()[index], 1.0f);
        if (clip.w <= 0.0f)
            continue;
        const glm::vec3 ndc = glm::vec3(clip) / clip.w;
        if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f)
            continue;
        labels.push_back({index, {viewportMin.x + (ndc.x * 0.5f + 0.5f) * size.x,
                                  viewportMin.y + (1.0f - (ndc.y * 0.5f + 0.5f)) * size.y}});
    }
    return labels;
}

} // namespace veometri::sculpt
