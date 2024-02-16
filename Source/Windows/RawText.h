#include "Base/EditorWindow.h"

class TextEditor : public EditorWindow
{
public:
    void OnImGuiDraw();

private:
    std::string myRawText;
    bool myHasTakenFocus;
};