#include "Base/EditorWindow.h"

class TextEditor : public EditorWindow
{
public:
    TextEditor();
    void OnImGuiDraw();

private:
    std::string myRawText;
    bool myHasTakenFocus;
};