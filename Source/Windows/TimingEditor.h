//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"

struct ImFont;
class TimingEditor : public EditorWindow
{
public:
    static TimingEditor& Get();

    TimingEditor();
    void OnImGuiDraw();
    void SetFont(ImFont* aFont, ImFont* aCustomFont = nullptr);
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
    bool GetInputUnsafe();
    void SetLatencyOffset(int someCentiSeconds);
    int GetLatencyOffset();

private:
    void DrawTextMarker();
    void DrawImagePopup();
    void DrawImageTagWidget(int aLine, int aToken);
    void DrawLineTagWidget(int aLine, int aToken);

    static inline TimingEditor* ourInstance;

    ImFont* myFont;
    ImFont* myCustomFont;
    int myMarkedLine = 0;
    int myMarkedToken = 0;
    int myMarkedChar = 0;
    bool myMarkHasMoved = false;
    bool myInputIsUnsafe = false;
    bool myDisableInput = false;
    int myLatencyOffset = 0;

    bool myIsImagePopupOpen = false;
    int myImagePopupEditLine = 0;
    std::string myImagePopupSelectedPath;
    int myImagePopupFadeStartShift = 0;
    int myImagePopupFadeDuration = 0;
};