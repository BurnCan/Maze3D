#pragma once

namespace mesh_sculpt::sculpt {

class MeshSculptUi
{
public:
    enum class FileAction
    {
        None,
        ResetMesh,
        Exit
    };

    FileAction renderMainMenuBar();
    void renderInfoPanel(bool cameraControl) const;
};

} // namespace mesh_sculpt::sculpt
