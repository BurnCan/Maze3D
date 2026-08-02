#pragma once

namespace mesh_sculpt::sculpt {

class MeshSculptTool;

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
    void renderToolPanel(MeshSculptTool& tool) const;
};

} // namespace mesh_sculpt::sculpt
