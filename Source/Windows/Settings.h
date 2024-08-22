//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE and LICENSE-SUPPLEMENT or <https://github.com/98ahni/Resonate>)
//  <Copyright (C) 2024 98ahni> Original file author

#include "Base/EditorWindow.h"
#include <string>

class Settings : public EditorWindow
{
public:
    Settings();
    void OnImGuiDraw();

private:
    int DrawLatencyWidget();

    bool myLatencyPopup;
    bool myLatencyPopupOpenLastFrame;
    float myLatencyStartTime;
};