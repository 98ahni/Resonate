//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <Extensions/History.h>

class TextEditor : public EditorWindow
{
    struct TextHistory : public History::Record
    {
        std::string mySavedRawText;
    public:
        TextHistory(std::string aRawText);
        void Undo() override;
        void Redo() override;
    };
public:
    TextEditor();
    void OnImGuiDraw();

private:
    std::string myRawText;
    bool myHasTakenFocus;
    double myLastEditTime;
    bool myShouldSerialize;
    static inline bool ourShouldReload;
};