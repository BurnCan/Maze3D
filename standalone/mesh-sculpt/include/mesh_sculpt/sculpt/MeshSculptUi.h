#pragma once

namespace mesh_sculpt::sculpt {

class MeshSculptUi
{
public:
    enum class FileAction
    {
        None,
        NewDocument,
        OpenDocument,
        SaveDocument,
        SaveDocumentAs,
        ResetMesh,
        Exit
    };

    FileAction renderMainMenuBar();
    void renderInfoPanel(bool cameraControl) const;
};

} // namespace mesh_sculpt::sculpt
