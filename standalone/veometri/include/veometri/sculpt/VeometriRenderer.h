#pragma once

#include <memory>

#include "veometri/sculpt/MeshSelection.h"
#include "veometri/sculpt/SculptMesh.h"

namespace veometri::render { class Camera; }

namespace veometri::sculpt {

class VeometriRenderer
{
public:
    // The camera is non-owning and must outlive this renderer. A null camera
    // is accepted for the tool's existing headless/no-camera behavior.
    explicit VeometriRenderer(const render::Camera* camera);
    ~VeometriRenderer();

    VeometriRenderer(const VeometriRenderer&) = delete;
    VeometriRenderer& operator=(const VeometriRenderer&) = delete;

    void upload(const SculptMesh& mesh);
    void render(const SculptMesh& mesh, const MeshSelection& selection);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace veometri::sculpt
