#include "Base/EditorWindow.h"

struct ImFont;
class TimingEditor : public EditorWindow
{
public:
    void OnImGuiDraw();
    void SetFont(ImFont* aFont);
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
    void SetInputUnsafe(bool anUnsafe);

private:
    void DrawTextMarker();

    ImFont* myFont;
    int myMarkedLine = 0;
    int myMarkedToken = 0;
    int myMarkedChar = 0;
    bool myInputIsUnsafe = false;
    bool myDisableInput = false;
};