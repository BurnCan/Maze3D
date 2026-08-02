#pragma once

#include <string>
#include <utility>

namespace veometri::sculpt {

class SculptMesh;

class MeshEditorState
{
public:
    const std::string& verticesText() const noexcept { return m_verticesText; }
    const std::string& indicesText() const noexcept { return m_indicesText; }
    const std::string& errorMessage() const noexcept { return m_errorMessage; }

    std::string& editVerticesText() noexcept { return m_verticesText; }
    std::string& editIndicesText() noexcept { return m_indicesText; }
    void synchronizeVertices(const SculptMesh& mesh);
    void synchronizeIndices(const SculptMesh& mesh);
    void synchronizeAll(const SculptMesh& mesh);
    void setError(std::string message) { m_errorMessage = std::move(message); }
    void clearError() noexcept { m_errorMessage.clear(); }

private:
    std::string m_verticesText;
    std::string m_indicesText;
    std::string m_errorMessage;
};

} // namespace veometri::sculpt
