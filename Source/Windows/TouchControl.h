#include "Base/EditorWindow.h"

class TimingEditor;
class TouchControl : public EditorWindow
{
public:
    void OnImGuiDraw() override;

private:
    bool myIsCharMode = false;
    TimingEditor* myEditor = nullptr;
    float myLastXSize = 50;
};