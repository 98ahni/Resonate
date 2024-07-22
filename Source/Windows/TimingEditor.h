//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"

struct ImFont;
class TimingEditor : public EditorWindow
{
public:
    TimingEditor();
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
    void CheckMarkerIsSafe(bool aIsMovingRight);
    void SetInputUnsafe(bool anUnsafe);
    void SetLatencyOffset(int someCentiSeconds);
    int GetLatencyOffset();

private:
    void DrawTextMarker();

    ImFont* myFont;
    int myMarkedLine = 0;
    int myMarkedToken = 0;
    int myMarkedChar = 0;
    bool myMarkHasMoved = false;
    bool myInputIsUnsafe = false;
    bool myDisableInput = false;
    int myLatencyOffset = 0;
};