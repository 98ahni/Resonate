#include "Base/EditorWindow.h"

class TimingEditor : public EditorWindow
{
public:
    void OnImGuiDraw();
    int GetMarkedLine();
    int GetMarkedToken();
    int GetMarkedChar();
    void ToggleTokenHasTime();
    void RecordStartTime();
    void RecordEndTime();
    void MoveMarkerUp();
    void MoveMarkerDown();
    void MoveMarkerLeft(bool aIsCharmode = false);
    void MoveMarkerRight(bool aIsCharmode = false);
    void DisableInput(bool aDisable);

private:
    void DrawTextMarker();

    int myMarkedLine = 0;
    int myMarkedToken = 0;
    int myMarkedChar = 0;
    bool myDisableInput = false;
};