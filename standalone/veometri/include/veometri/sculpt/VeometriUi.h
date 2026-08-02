#pragma once

namespace veometri::sculpt {

class VeometriUi
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

} // namespace veometri::sculpt
