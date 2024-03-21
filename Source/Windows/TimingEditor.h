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
    void MoveMarkerLeft();
    void MoveMarkerRight();

private:
    void DrawTextMarker();

    int myMarkedLine = 0;
    int myMarkedToken = 0;
    int myMarkedChar = 0;
};