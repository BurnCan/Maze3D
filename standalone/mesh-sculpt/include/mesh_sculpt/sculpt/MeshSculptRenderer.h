#pragma once

#include <memory>

#include "mesh_sculpt/sculpt/MeshSelection.h"
#include "mesh_sculpt/sculpt/SculptMesh.h"

namespace mesh_sculpt::render { class Camera; }

namespace mesh_sculpt::sculpt {

class MeshSculptRenderer
{
public:
    // The camera is non-owning and must outlive this renderer. A null camera
    // is accepted for the tool's existing headless/no-camera behavior.
    explicit MeshSculptRenderer(const render::Camera* camera);
    ~MeshSculptRenderer();

    MeshSculptRenderer(const MeshSculptRenderer&) = delete;
    MeshSculptRenderer& operator=(const MeshSculptRenderer&) = delete;

    void upload(const SculptMesh& mesh);
    void render(const SculptMesh& mesh, const MeshSelection& selection);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace mesh_sculpt::sculpt
