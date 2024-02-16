#include "Base/EditorWindow.h"

class TimingEditor : public EditorWindow
{
public:
    void OnImGuiDraw();

private:
    void DrawTextMarker();

    int myMarkedLine = 0;
    int myMarkedToken = 0;
    int myMarkedChar = 0;
};