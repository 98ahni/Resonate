//  This file is licenced under the GNU General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

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