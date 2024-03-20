#include "Base/EditorWindow.h"

class TouchControl : public EditorWindow
{
public:
    void OnImGuiDraw() override;

private:
    bool myIsCharMode = false;
};